#include "tasks.h"
#include "esp_dsp.h"
#include "sigproc.h"

// Using 3 coeffs only. The 4th is used to memory alignment.
#define COEFFS_LEN 4
#define SIGNAL_FREQUENCY 25.0f

// Decl
void task_preprocess(void *params);
float slow_activity_level(float al);
float activity_level(const float *const signal, int len);
void aggregate_acc_window(const float *const w1, const float *const w2, const float *const w3, float *output, int len);

float alternative_al(const float *const signal, int len);
float trapz_al_raw(const float *const signal, int len);
float trapz_al_norm(const float *const signal, int len);

// Impl
void task_preprocess(void *params)
{
    // static __attribute__((aligned(16))) float fir_coeffs[COEFFS_LEN] = {-0.05637224f, 0.9087878f, -0.05637224f, 0.0f};
    static __attribute__((aligned(16))) float fir_coeffs[COEFFS_LEN] = {-0.5f, 1.0f, -0.5f, 0.0f};
    static __attribute__((aligned(16))) float temp_input[3][WINDOW_LEN_ALIGNMENT];

    static __attribute__((aligned(16))) float delay_line_x[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) float delay_line_y[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) float delay_line_z[COEFFS_LEN + 4];

    fir_f32_t fir_x, fir_y, fir_z;
    buffer_t *buffer = NULL;
    float agg_signal[WINDOW_LEN] = {0};

    // Initialize fir filter
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_x, fir_coeffs, delay_line_x, COEFFS_LEN));
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_y, fir_coeffs, delay_line_y, COEFFS_LEN));
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_z, fir_coeffs, delay_line_z, COEFFS_LEN));

    // TODO: Checks if prints trash
    ESP_LOGI("PREPROCESS", "Filters initialized successfully");

    // Task loop
    while (1)
    {
        // Receive raw data
        xQueueReceive(raw_data_queue, &buffer, portMAX_DELAY);
        
        // Auxiliar input buffer
        for (int i = 0; i < 3; i++)
        {
            memcpy(temp_input[i], buffer->acc[i], WINDOW_LEN * sizeof(float));
        }

        // Filtering
        dsps_fir_f32_aes3(&fir_x, temp_input[0], buffer->acc[0], WINDOW_LEN);
        dsps_fir_f32_aes3(&fir_y, temp_input[1], buffer->acc[1], WINDOW_LEN);
        dsps_fir_f32_aes3(&fir_z, temp_input[2], buffer->acc[2], WINDOW_LEN);

        // Aggregating
        aggregate_acc_window(buffer->acc[0], buffer->acc[1], buffer->acc[2], agg_signal, WINDOW_LEN);

        // // DEBUG
        // for (int x = 0; x < WINDOW_LEN; x++)
        // {
        //     printf("%f,", agg_signal[x]);
        // }
        // printf("\n");

        // AL raw calc
        float al_raw = trapz_al_raw(agg_signal, WINDOW_LEN);
        buffer->al_raw = al_raw;

        // AL norm
        clip(agg_signal, WINDOW_LEN, 1.0f);
        float al = trapz_al_norm(agg_signal, WINDOW_LEN);

        // EWMA
        // float al_raw = activity_level(agg_signal, WINDOW_LEN);
        // buffer->al_raw = al_raw;

        // Calculating ALnorm by clipped acc data
        // clip(agg_signal, WINDOW_LEN, 1.0f);

        // float al = activity_level(agg_signal, WINDOW_LEN);
        // float al = alternative_al(agg_signal, WINDOW_LEN);
        // buffer->al_raw = al;

#ifdef CONFIG_EXPONENTIAL_APPROXIMATION_MODEL
        buffer->al = al;
#else
        buffer->als[0] = al;
        buffer->als[1] = al * al;
        buffer->als[2] = slow_activity_level(al);
#endif

        // Send data to the next stage
        xQueueSend(filtered_data_queue, &buffer, portMAX_DELAY);
    }
}





// Needs to repeat code to hold different states
float trapz_al_norm(const float *const signal, int len)
{
    static trapz_ctx_t ctx;
    static int init_flag = 0;

    if (!init_flag)
    {
        ctx.dt = 1.0f / SIGNAL_FREQUENCY;
        ctx.prev = 0.0f;
        init_flag = 1;
    }

    float al = trapz_integral(&ctx, signal, len);

    float window_time = (float)len / SIGNAL_FREQUENCY;
    al /= window_time;

    return al;
}

float trapz_al_raw(const float *const signal, int len)
{
    static trapz_ctx_t ctx;
    static int init_flag = 0;

    if (!init_flag)
    {
        ctx.dt = 1.0f / SIGNAL_FREQUENCY;
        ctx.prev = 0.0f;
        init_flag = 1;
    }

    float al = trapz_integral(&ctx, signal, len);
    return al;
}

// Calculates mean of agg window
float signal_mean(const float *const signal, int len)
{
    float mean = 0;

    for (int i = 0; i < len; i++)
    {
        mean += signal[i];
    }
    mean /= len;
    return mean;
}

// EWMA AL - block mean
float alternative_al(const float *const signal, int len)
{
    static ewma_t ewma_filter;
    static int init_flag = 0;

    float mean = signal_mean(signal, len);

    if (!init_flag)
    {
        ewma_filter.alpha = 0.25;
        ewma_filter.last_value = mean;
        init_flag = 1;
    }

    ewma_update(&ewma_filter, mean);

    return ewma_filter.last_value;
}

// EWMA AL - all samples
float activity_level(const float *const signal, int len)
{
    static ewma_t ewma_filter;
    static int init_flag = 0;

    if (!init_flag)
    {
        // ewma_filter.alpha = 0.0769;
        ewma_filter.alpha = 0.05;

        ewma_filter.last_value = signal[0];
        init_flag = 1;
    }

    for (int i = 0; i < len; i++)
    {
        ewma_update(&ewma_filter, signal[i]);
    }
    return ewma_filter.last_value;

    // ewma_t ewma_filter = {
    // .alpha = 0.05, Value used for best results until now
    // .alpha = 0.01586 //-> 1s window
    // .alpha = 0.0392 // -> 2s window

    // .alpha = 0.0769, // Calculated manually for 25 samples
    // .last_value = signal[0]};
}

// EWMA AL - over AL
float slow_activity_level(float al)
{
    static ewma_t ewma_filter;
    static int init_flag = 0;

    if (!init_flag)
    {
        ewma_filter.alpha = 0.05;
        ewma_filter.last_value = al;
        init_flag = 1;
    }

    ewma_update(&ewma_filter, al);

    return ewma_filter.last_value;
}

// =====================================================
void aggregate_acc_window(const float *const w1, const float *const w2, const float *const w3, float *output, int len)
{
    for (int i = 0; i < len; i++)
    {
        acc_t sig = {
            .x = w1[i],
            .y = w2[i],
            .z = w3[i]};

        output[i] = acc_aggregation(&sig);
    }
}
