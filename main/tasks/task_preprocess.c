#include "tasks.h"
#include "esp_dsp.h"
#include "sigproc.h"
#include "comm_protocol.h"

// Using 3 coeffs only. The 4th is used to memory alignment.
#define COEFFS_LEN 4

void task_preprocess(void *params)
{
    fir_f32_t fir_x, fir_y, fir_z;
    buffer_t *buffer = NULL;

    trapz_ctx_t al_raw_ctx = {.dt = 1.0f / (float)WINDOW_LEN, .prev = 0.0f};
    trapz_ctx_t al_norm_ctx = {.dt = 1.0f / (float)WINDOW_LEN, .prev = 0.0f};

    // =================
    // EWMA alternative
    //==================
    // ewma_t al_raw_ctx = {.alpha = 0.0769, .last_value = 0.0f};
    // ewma_t al_norm_ctx = {.alpha = 0.0769, .last_value = 0.0f};

    static __attribute__((aligned(16))) float acc_mag[WINDOW_LEN_ALIGNMENT];

    // FIR circuit buffers
    static __attribute__((aligned(16))) float delay_line_x[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) float delay_line_y[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) float delay_line_z[COEFFS_LEN + 4];

    // Order 2 high-pass FIR coefficients
    static __attribute__((aligned(16))) float fir_coeffs[COEFFS_LEN] = {-0.5f, 1.0f, -0.5f, 0.0f};

    // FIR init
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_x, fir_coeffs, delay_line_x, COEFFS_LEN));
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_y, fir_coeffs, delay_line_y, COEFFS_LEN));
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir_z, fir_coeffs, delay_line_z, COEFFS_LEN));

    esp_cpu_cycle_count_t start_cycles, end_cycles;
    float elapsed_time;

    while (1)
    {
        // Receive raw data
        if (xQueueReceive(raw_data_queue, &buffer, portMAX_DELAY) == pdTRUE)
        {
            // cycle count
            start_cycles = esp_cpu_get_cycle_count();

            // In-place filtering
            dsps_fir_f32_aes3(&fir_x, buffer->acc[0], buffer->acc[0], WINDOW_LEN);
            dsps_fir_f32_aes3(&fir_y, buffer->acc[1], buffer->acc[1], WINDOW_LEN);
            dsps_fir_f32_aes3(&fir_z, buffer->acc[2], buffer->acc[2], WINDOW_LEN);

            // Acc magnitude
            calc_accel_mag_vec(
                buffer->acc[0],
                buffer->acc[1],
                buffer->acc[2],
                acc_mag,
                WINDOW_LEN);

            // AL raw calc
            buffer->al_raw = trapz_integral(&al_raw_ctx, acc_mag, WINDOW_LEN);

            // AL norm calc
            clip(acc_mag, WINDOW_LEN, 1.0f);
            buffer->al = trapz_integral(&al_norm_ctx, acc_mag, WINDOW_LEN);

            //==================================================
            // Activity Level calculated with EWMA (alternative)
            //==================================================
            // for(int i=0; i<WINDOW_LEN; i++) {
            //     ewma_update(&al_raw_ctx, acc_mag[i]);
            // }
            // buffer->al_raw = al_raw_ctx.last_value;

            // clip(acc_mag, WINDOW_LEN, 1.0f);
            // for(int i=0; i<WINDOW_LEN; i++) {
            //     ewma_update(&al_norm_ctx, acc_mag[i]);
            // }
            // buffer->al = al_norm_ctx.last_value;

            end_cycles = esp_cpu_get_cycle_count();
            elapsed_time = (float)(end_cycles - start_cycles) / (float)CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;

            char log[64];
            snprintf(log, sizeof(log), "[PRE-PROCESS] Elapsed time: %.2fus", elapsed_time);
            comm_send_packet(PKT_TYPE_LOG, (uint8_t *)log, strlen(log));

            // Send data to the next stage
            xQueueSend(filtered_data_queue, &buffer, portMAX_DELAY);
        }
        else
        {
            // TODO: log
        }
    }
}
