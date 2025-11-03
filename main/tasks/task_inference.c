#include "tasks.h"
#include <stdbool.h>
#include "exp_approx_model.h"

// Just for tests
#define BMI 23.82f
#define AGE 26.0f

void task_inference(void *params){

    buffer_t * bf = NULL;

    eam_t model;
    ea_model_init(&model);
    ea_model_set_user_data(&model, BMI, MALE, AGE);

    while(1) {
        xQueueReceive(filtered_data_queue, &bf, portMAX_DELAY);

        printf("INFERENCE - Activity Level = %f\n", bf->al);

        ea_model_partial_fit(&model, bf->al);
        ea_model_predict(&model);

        bf->hr = (int)model.next_hr;

        ea_model_debug(&model);

        xQueueSend(inference_result_queue, &bf, portMAX_DELAY);
    }
}
