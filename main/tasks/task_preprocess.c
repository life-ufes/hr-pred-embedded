#include "tasks.h"
#include "esp_dsp.h"
#include "sigproc.h"
#include "comm_protocol.h"

// Using 3 coeffs only. The 4th is used to memory alignment.
#define COEFFS_LEN 4

void task_preprocess(void *params)
{
    fir_f32_t fir[RAW_SIGNAL_CHANNELS];
    buffer_t *buffer = NULL;

    trapz_ctx_t acc_raw_ctx = {.dt = 1.0f / (float)WINDOW_LEN, .prev = 0.0f};
    trapz_ctx_t acc_norm_ctx = {.dt = 1.0f / (float)WINDOW_LEN, .prev = 0.0f};
    trapz_ctx_t gyro_raw_ctx = {.dt = 1.0f / (float)WINDOW_LEN, .prev = 0.0f};
    trapz_ctx_t gyro_norm_ctx = {.dt = 1.0f / (float)WINDOW_LEN, .prev = 0.0f};

    // =================
    // EWMA alternative
    //==================
    // ewma_t al_raw_ctx = {.alpha = 0.0769, .last_value = 0.0f};
    // ewma_t al_norm_ctx = {.alpha = 0.0769, .last_value = 0.0f};

    static __attribute__((aligned(16))) float acc_mag[WINDOW_LEN_ALIGNMENT];
    static __attribute__((aligned(16))) float gyro_mag[WINDOW_LEN_ALIGNMENT];

    // FIR circuit buffers
    static __attribute__((aligned(16))) float delay_lines[RAW_SIGNAL_CHANNELS][COEFFS_LEN + 4];

    // Order 2 high-pass FIR coefficients
    static __attribute__((aligned(16))) float fir_coeffs[COEFFS_LEN] = {-0.5f, 1.0f, -0.5f, 0.0f};

    // FIR init
    for (int i = 0; i < RAW_SIGNAL_CHANNELS; i++)
    {
        ESP_ERROR_CHECK(dsps_fir_init_f32(&fir[i], fir_coeffs, delay_lines[i], COEFFS_LEN));
    }

    esp_cpu_cycle_count_t start_cycles, end_cycles;
    float elapsed_time;

    while (1)
    {
        // Receive raw data from the previous stage
        if (xQueueReceive(raw_data_queue, &buffer, portMAX_DELAY) == pdTRUE)
        {
            // cycle count
            start_cycles = esp_cpu_get_cycle_count();

            // In-place filtering
            for (int axis = 0; axis < SENSOR_AXES; axis++)
            {
                dsps_fir_f32_aes3(&fir[axis], buffer->acc[axis], buffer->acc[axis], WINDOW_LEN);
                dsps_fir_f32_aes3(&fir[SENSOR_AXES + axis], buffer->gyro[axis], buffer->gyro[axis], WINDOW_LEN);
            }

            // Acc magnitude
            calc_accel_mag_vec(
                buffer->acc[0],
                buffer->acc[1],
                buffer->acc[2],
                acc_mag,
                WINDOW_LEN);

            // Gyro magnitude
            calc_accel_mag_vec(
                buffer->gyro[0],
                buffer->gyro[1],
                buffer->gyro[2],
                gyro_mag,
                WINDOW_LEN);

            // Acc AL calc
            float acc_al_raw = trapz_integral(&acc_raw_ctx, acc_mag, WINDOW_LEN);
            clip(acc_mag, WINDOW_LEN, 1.0f);
            float acc_al = trapz_integral(&acc_norm_ctx, acc_mag, WINDOW_LEN);

            // Gyro AL calc
            float gyro_al_raw = trapz_integral(&gyro_raw_ctx, gyro_mag, WINDOW_LEN);
            clip(gyro_mag, WINDOW_LEN, 1.0f);
            float gyro_al = trapz_integral(&gyro_norm_ctx, gyro_mag, WINDOW_LEN);

            // Weighted fusion into the final AL value
            const float acc_weight = 0.5f;
            const float gyro_weight = 0.5f;
            buffer->al_raw = (acc_weight * acc_al_raw) + (gyro_weight * gyro_al_raw);
            buffer->al = (acc_weight * acc_al) + (gyro_weight * gyro_al);

            // Keep the fused value bounded for inference stability
            
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
