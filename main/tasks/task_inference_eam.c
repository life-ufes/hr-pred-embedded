#include "tasks.h"
#include <stdbool.h>
#include "exp_approx_model.h"
#include "comm_protocol.h"

// Just for tests
#define BMI 20.7f
#define AGE 26.9f
#define MALE 0.52f
#define FEMALE 0.42f

void task_inference_eam(void *params)
{
    buffer_t *bf = NULL;

    // Setup model
    eam_t model;
    ea_model_init(&model);
    ea_model_set_user_data(&model, BMI, AGE, MALE, FEMALE);

    // HR adjustment flag
    int set_start_hr_train = 0;

    esp_cpu_cycle_count_t start_cycles, end_cycles;
    float elapsed_time;

    while (1)
    {
        if (xQueueReceive(filtered_data_queue, &bf, portMAX_DELAY) == pdTRUE)
        {
            // cycle count
            start_cycles = esp_cpu_get_cycle_count();

            // Set info from buffer
            ea_model_set_al(&model, bf->al);
            ea_model_handle_intensity(&model, bf->al_raw);

            // HR adjustment
            if (bf->train)
            {
                if (!set_start_hr_train)
                {
                    model.next_hr = bf->hr_gt;
                    set_start_hr_train = 1;
                }
                ea_model_partial_fit(&model, bf->hr_gt);
            }
            else
            {
                set_start_hr_train = 0;
            }

            // Inference
            ea_model_predict(&model);
            bf->hr = model.next_hr;
            bf->hr_reg = model.hr_reg;

            // Exec time calc
            end_cycles = esp_cpu_get_cycle_count();
            elapsed_time = (float)(end_cycles - start_cycles) / (float)CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;

            // LOGS
            char log[64];
            snprintf(log, sizeof(log), "[PRE-INFERENCE] Elapsed time: %.2fus", elapsed_time);
            comm_send_packet(PKT_TYPE_LOG, (uint8_t *)log, strlen(log));

            // Sends to the next stage
            xQueueSend(inference_result_queue, &bf, portMAX_DELAY);
        }
        else
        {
            // TODO: log
        }
    }
}
