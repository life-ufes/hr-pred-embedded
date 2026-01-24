#include "tasks.h"
#include <stdbool.h>
#include "exp_approx_model.h"
#include "comm_protocol.h"

// Just for tests
#define BMI 22.7f
#define AGE 26.9f

// TODO: apply gender by %
// Receive this daa from serial

void task_inference_eam(void *params)
{

    buffer_t *bf = NULL;

    eam_t model;
    ea_model_init(&model);
    ea_model_set_user_data(&model, BMI, MALE, AGE);

    int set_start_hr_train = 0;
    int set_start_hr_test = 0;

    esp_cpu_cycle_count_t start_cycles, end_cycles, cycles_elapsed;
    float elapsed_time;

    while (1)
    {
        xQueueReceive(filtered_data_queue, &bf, portMAX_DELAY);

        start_cycles = esp_cpu_get_cycle_count();

        ea_model_set_al(&model, bf->al);
        ea_model_handle_intensity(&model, bf->al_raw);

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

        ea_model_predict(&model);

        // TODO: improve this logic
        // else if (!bf->train && !set_start_hr_test)
        // {
        //     model.next_hr = bf->hr_gt;
        //     set_start_hr_train = 0;
        //     set_start_hr_test = 1;
        // }

        bf->hr = model.next_hr;

        // TODO: Prints results by UART in next task
        // TODO: rename send and receive tasks
        // ea_model_debug(&model, bf->hr_gt);


        end_cycles = esp_cpu_get_cycle_count();
        cycles_elapsed = end_cycles - start_cycles;
        elapsed_time = (float)cycles_elapsed / (float)CONFIG_ESP_DEFAULT_CPU_FREQ_MHZ;

        char log[64]; 
        snprintf(log, sizeof(log), "[PRE-INFERENCE] Elapsed time: %.2fus", elapsed_time);
        comm_send_packet(PKT_TYPE_LOG, (uint8_t *)log, strlen(log));

        xQueueSend(inference_result_queue, &bf, portMAX_DELAY);
    }
}
