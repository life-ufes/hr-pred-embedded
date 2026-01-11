#include "tasks.h"
#include "dynamic_exp_model.h"

// Just for tests
#define BMI 23.82f
#define AGE 26.0f

void task_inference_dem(void *params)
{
    buffer_t * bf = NULL;

    dem_t model;
    de_model_init(&model);
    de_model_set_user_data(&model, BMI, MALE, AGE);

    while(1) {
        xQueueReceive(filtered_data_queue, &bf, portMAX_DELAY);

        de_model_set_als(&model, bf->als);
        de_model_handle_intensity(&model);
        de_model_handle_phase(&model);

        if(bf->train){
            de_model_partial_fit(&model, bf->hr_gt);
        }
        
        de_model_predict(&model);
        bf->hr = model.next_hr;

        de_model_debug(&model, bf->hr_gt);

        xQueueSend(inference_result_queue, &bf, portMAX_DELAY);
    }
}
