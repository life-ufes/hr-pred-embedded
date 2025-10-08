#include "tasks.h"

void task_inference(void *params){
    
    float * buffer = NULL;
    while(1) {
        xQueueReceive(filtered_data_queue, &buffer, portMAX_DELAY);

        printf("FILTERED DATA: \n");
        for(int x=0; x<SIGNAL_LEN; x++) {
            printf("%f ", buffer[x]);
        }
        printf("\n");

        xQueueSend(buffer_pool_queue, &buffer, portMAX_DELAY);
    }
}
