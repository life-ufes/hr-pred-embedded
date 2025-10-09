#include "tasks.h"

void task_receive(void *params)
{
    // float *buffer = NULL;

    buffer_t * bf;

    // Simulating data receive
    float data[SIGNAL_LEN * 3] = {0};
    for (int x = 0; x < (SIGNAL_LEN * 3); x++)
    {
        data[x] = (float)x;
    }

    ESP_LOGI("RECEIVE TASK", "Task intialized succesfully.");


    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 to 1 second sends data to the next task

        xQueueReceive(buffer_pool_queue, &bf, portMAX_DELAY); // Catching an available buffer
        
        if(!bf) {
            ESP_LOGW("receive_buffer", "NULL buffer");
            continue;
        }
        
        memcpy(bf->acc, data, SIGNAL_LEN * 3 * sizeof(float));


        xQueueSend(raw_data_queue, &bf, portMAX_DELAY);
    }
}
