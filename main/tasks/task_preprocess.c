#include "tasks.h"
#include "esp_dsp.h"
#include "sigproc.h"
#include "comm_protocol.h"

// Using 3 coeffs only. The 4th is used to memory alignment.
#define COEFFS_LEN 4

// LSM6DSM +-8g -> 0.244 mg/LSB
// Convertion factor
#define SENSITIVITY_8G 0.000244f

void task_preprocess(void *params)
{
    // s16 fir
    fir_s16_t fir_x, fir_y, fir_z;
    buffer_t *buffer = NULL;

    trapz_ctx_t al_raw_ctx = {.dt = 1.0f / (float)WINDOW_LEN, .prev = 0.0f};
    trapz_ctx_t al_norm_ctx = {.dt = 1.0f / (float)WINDOW_LEN, .prev = 0.0f};

    static __attribute__((aligned(16))) float acc_mag[WINDOW_LEN_ALIGNMENT];

    // Delay line s16
    static __attribute__((aligned(16))) int16_t delay_line_x[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) int16_t delay_line_y[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) int16_t delay_line_z[COEFFS_LEN + 4];

    //  Quantized coeff Q15 for {-0.5, 1.0, -0.5, 0.0}
    static __attribute__((aligned(16))) int16_t fir_coeffs_q15[COEFFS_LEN] = {-16384, 32767, -16384, 0};

    ESP_ERROR_CHECK(dsps_fird_init_s16(&fir_x, fir_coeffs_q15, delay_line_x, COEFFS_LEN, 1, 0, 15));
    ESP_ERROR_CHECK(dsps_fird_init_s16(&fir_y, fir_coeffs_q15, delay_line_y, COEFFS_LEN, 1, 0, 15));
    ESP_ERROR_CHECK(dsps_fird_init_s16(&fir_z, fir_coeffs_q15, delay_line_z, COEFFS_LEN, 1, 0, 15));

    esp_cpu_cycle_count_t start_cycles, end_cycles;
    float elapsed_time;

    while (1)
    {
        // Receive raw data from the previous stage
        if (xQueueReceive(raw_data_queue, &buffer, portMAX_DELAY) == pdTRUE)
        {
            start_cycles = esp_cpu_get_cycle_count();

            // 5. In-place filtering using Q15
            dsps_fird_s16_aes3(&fir_x, buffer->raw_acc[0], buffer->raw_acc[0], WINDOW_LEN);
            dsps_fird_s16_aes3(&fir_y, buffer->raw_acc[1], buffer->raw_acc[1], WINDOW_LEN);
            dsps_fird_s16_aes3(&fir_z, buffer->raw_acc[2], buffer->raw_acc[2], WINDOW_LEN);

            // Upscaling
            for (int i = 0; i < WINDOW_LEN; i++) {
                buffer->acc[0][i] = (float)buffer->raw_acc[0][i] * SENSITIVITY_8G;
                buffer->acc[1][i] = (float)buffer->raw_acc[1][i] * SENSITIVITY_8G;
                buffer->acc[2][i] = (float)buffer->raw_acc[2][i] * SENSITIVITY_8G;
            }

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

            end_cycles = esp_cpu_get_cycle_count();
            elapsed_time = (float)(end_cycles - start_cycles) / (float)CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;
            UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
            
            buffer->pre_process_time = elapsed_time;
            buffer->pre_process_hwm = uxHighWaterMark;

            // Send data to the next stage
            xQueueSend(filtered_data_queue, &buffer, portMAX_DELAY);
        }
        else
        {
            // TODO: log
        }
    }
}