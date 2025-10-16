#include "tasks.h"
#include "esp_dsp.h"
#include "sigproc.h"

// Using 3 coeffs only. The 4th is used to memory alignment.
#define COEFFS_LEN 4

// Decl
void task_preprocess(void *params);
float activity_level(const float *const signal, int len);
void aggregate_acc_window(const float *const w1, const float *const w2, const float *const w3, float *output, int len);

// Impl
void task_preprocess(void *params)
{
    static __attribute__((aligned(16))) float fir_coeffs[COEFFS_LEN] = {0.05637224f, 0.9087878f, 0.05637224f, 0.0f};
    static __attribute__((aligned(16))) float delay_line[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) float temp_input[3][SIGNAL_LEN] = {{0}, {0}, {0}};

    fir_f32_t fir_x, fir_y, fir_z;
    buffer_t *buffer = NULL;
    float agg_signal[SIGNAL_LEN] = {0};
    
    // Initialize fir filter
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_x, fir_coeffs, delay_line, COEFFS_LEN));
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_y, fir_coeffs, delay_line, COEFFS_LEN));
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_z, fir_coeffs, delay_line, COEFFS_LEN));

    ESP_LOGI("PREPROCESS", "Filters initialized successfully");

    // Task loop
    while (1)
    {
        // Receive raw data
        xQueueReceive(raw_data_queue, &buffer, portMAX_DELAY);

        // Auxiliar input buffer
        for (int i = 0; i < 3; i++)
        {
            memcpy(temp_input[i], buffer->acc[i], SIGNAL_LEN * sizeof(float));
        }

        // Filtering
        dsps_fir_f32_aes3(&fir_x, temp_input[0], buffer->acc[0], SIGNAL_LEN);
        dsps_fir_f32_aes3(&fir_y, temp_input[1], buffer->acc[1], SIGNAL_LEN);
        dsps_fir_f32_aes3(&fir_z, temp_input[2], buffer->acc[2], SIGNAL_LEN);

        // Aggregating
        aggregate_acc_window(buffer->acc[0], buffer->acc[1], buffer->acc[2], agg_signal, SIGNAL_LEN);

        // EWMA
        buffer->al = activity_level(agg_signal, SIGNAL_LEN);

        // Send data to the next stage
        xQueueSend(filtered_data_queue, &buffer, portMAX_DELAY);
    }
}


// Activity level computing=============================
float activity_level(const float *const signal, int len)
{
    ewma_t ewma_filter = {
        .alpha = 0.0769, // Calculated manually
        .last_value = signal[0]};
    for (int i = 0; i < len; i++)
    {
        ewma_update(&ewma_filter, signal[i]);
    }
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
