#include "tasks.h"
#include "esp_dsp.h"
#include "sigproc.h"

// Using 3 coeffs only. The 4th is used to memory alignment.
#define COEFFS_LEN 4

// Decl
void task_preprocess(void *params);
void compute_signal_window_magnitude(
    const float *const w1,
    const float *const w2,
    const float *const w3,
    float *output,
    int len);

// Impl
void task_preprocess(void *params)
{
    fir_f32_t fir_x, fir_y, fir_z;
    buffer_t *buffer = NULL;

    trapz_ctx_t al_raw_ctx = {.dt = 1.0f / SIGNAL_FREQUENCY, .prev = 0.0f};
    trapz_ctx_t al_norm_ctx = {.dt = 1.0f / SIGNAL_FREQUENCY, .prev = 0.0f};

    // =================
    // EWMA alternative
    //==================
    // ewma_t al_raw_ctx = {.alpha = 0.0769, .last_value = 0.0f};
    // ewma_t al_norm_ctx = {.alpha = 0.0769, .last_value = 0.0f};

    float agg_signal[WINDOW_LEN] = {0};
    static __attribute__((aligned(16))) float temp_input[3][WINDOW_LEN_ALIGNMENT];

    // FIR params
    static __attribute__((aligned(16))) float delay_line_x[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) float delay_line_y[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) float delay_line_z[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) float fir_coeffs[COEFFS_LEN] = {-0.5f, 1.0f, -0.5f, 0.0f};

    // FIR init
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_x, fir_coeffs, delay_line_x, COEFFS_LEN));
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_y, fir_coeffs, delay_line_y, COEFFS_LEN));
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_z, fir_coeffs, delay_line_z, COEFFS_LEN));

    esp_cpu_cycle_count_t start_cycles, end_cycles, cycles_elapsed;
    float elapsed_time;

    while (1)
    {
        // Receive raw data
        xQueueReceive(raw_data_queue, &buffer, portMAX_DELAY);

        // cycle count
        start_cycles = esp_cpu_get_cycle_count();

        // Auxiliar input buffer
        for (int i = 0; i < 3; i++)
        {
            memcpy(temp_input[i], buffer->acc[i], WINDOW_LEN * sizeof(float));
        }

        // Filtering
        dsps_fir_f32_aes3(&fir_x, temp_input[0], buffer->acc[0], WINDOW_LEN);
        dsps_fir_f32_aes3(&fir_y, temp_input[1], buffer->acc[1], WINDOW_LEN);
        dsps_fir_f32_aes3(&fir_z, temp_input[2], buffer->acc[2], WINDOW_LEN);

        // Norm from axes
        compute_signal_window_magnitude(
            buffer->acc[0],
            buffer->acc[1],
            buffer->acc[2],
            agg_signal,
            WINDOW_LEN);

        // AL raw calc
        buffer->al_raw = trapz_integral(&al_raw_ctx, agg_signal, WINDOW_LEN);

        // AL norm calc
        clip(agg_signal, WINDOW_LEN, 1.0f);
        buffer->al = trapz_integral(&al_norm_ctx, agg_signal, WINDOW_LEN);

        //==================================================
        // Activity Level calculated with EWMA (alternative)
        //==================================================
        // for(int i=0; i<WINDOW_LEN; i++) {
        //     ewma_update(&al_raw_ctx, agg_signal[i]);
        // }
        // buffer->al_raw = al_raw_ctx.last_value;

        // clip(agg_signal, WINDOW_LEN, 1.0f);
        // for(int i=0; i<WINDOW_LEN; i++) {
        //     ewma_update(&al_norm_ctx, agg_signal[i]);
        // }
        // buffer->al = al_norm_ctx.last_value;

        end_cycles = esp_cpu_get_cycle_count();
        cycles_elapsed = end_cycles - start_cycles;
        elapsed_time = (float)cycles_elapsed / (float)CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;
        // TODO: Send elapsed time to buffer

        // Send data to the next stage
        xQueueSend(filtered_data_queue, &buffer, portMAX_DELAY);
    }
}

// =====================================================
void compute_signal_window_magnitude(
    const float *const w1,
    const float *const w2,
    const float *const w3,
    float *output,
    int len)
{
    for (int i = 0; i < len; i++)
    {
        output[i] = magnitude(w1[i], w2[i], w3[i]);
    }
}
