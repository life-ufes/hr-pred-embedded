#include "tasks.h"
#include <stdbool.h>
#include "exp_approx_model.h"

// Just for tests
#define BMI 22.7f
#define AGE 26.9f

// TODO: apply gender by %

void task_inference_eam(void *params){

    buffer_t * bf = NULL;

    eam_t model;
    ea_model_init(&model);
    ea_model_set_user_data(&model, BMI, MALE, AGE);

    int set_start_hr_train = 0;
    int set_start_hr_test = 0;

    while(1) {
        xQueueReceive(filtered_data_queue, &bf, portMAX_DELAY);

        // printf("INFERENCE TASK - Activity Level: %f\n HR Ground Truth: %f\n ", bf->al, bf->hr_gt);

        ea_model_set_al(&model, bf->al);
        ea_model_handle_intensity(&model, bf->al_raw);

        if(bf->train){
            ea_model_partial_fit(&model, bf->hr_gt);
        }
        
        ea_model_predict(&model);

        // TODO: improve this logic
        if (bf->train && !set_start_hr_train)
        {
            model.next_hr = bf->hr_gt;
            set_start_hr_train = 1;
            set_start_hr_test = 0;
        }
        else if (!bf->train && !set_start_hr_test)
        {
            model.next_hr = bf->hr_gt;
            set_start_hr_train = 0;
            set_start_hr_test = 1;
        }

        bf->hr = model.next_hr;

        ea_model_debug(&model, bf->hr_gt);

        xQueueSend(inference_result_queue, &bf, portMAX_DELAY);
    }
}
