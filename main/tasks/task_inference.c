#include "tasks.h"
#include <stdbool.h>


void task_inference(void *params){

    buffer_t * bf = NULL;

    while(1) {
        xQueueReceive(filtered_data_queue, &bf, portMAX_DELAY);

        printf("INFERENCE - Activity Level = %f\n", bf->al);

        xQueueSend(buffer_pool_queue, &bf, portMAX_DELAY);
    }
}
