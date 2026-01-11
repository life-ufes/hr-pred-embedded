#include "tasks.h"
#include <stdbool.h>
#include "exp_approx_model.h"

// Just for tests
#define BMI 23.82f
#define AGE 26.0f

void task_inference_eam(void *params){

    buffer_t * bf = NULL;

    eam_t model;
    ea_model_init(&model);
    ea_model_set_user_data(&model, BMI, MALE, AGE);

    while(1) {
        xQueueReceive(filtered_data_queue, &bf, portMAX_DELAY);

        // printf("INFERENCE TASK - Activity Level: %f\n HR Ground Truth: %f\n ", bf->al, bf->hr_gt);

        ea_model_set_al(&model, bf->al);
        ea_model_handle_intensity(&model);

        if(bf->train){
            ea_model_partial_fit(&model, bf->hr_gt);
        }
        
        ea_model_predict(&model);

        bf->hr = model.next_hr;

        ea_model_debug(&model, bf->hr_gt);

        xQueueSend(inference_result_queue, &bf, portMAX_DELAY);
    }
}
