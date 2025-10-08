#include "tasks.h"

void task_receive(void *params)
{
    float *buffer = NULL;

    // Simulating data receive
    float data[SIGNAL_LEN] = {0};
    for (int x = 0; x < SIGNAL_LEN; x++)
    {
        data[x] = (float)x;
    }

    ESP_LOGI("RECEIVE TASK", "Task intialized succesfully.");

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000)); // 1 to 1 second sends data to the next task

        xQueueReceive(buffer_pool_queue, &buffer, portMAX_DELAY); // Catching an available buffer
        
        if(!buffer) {
            ESP_LOGW("receive_buffer", "NULL buffer");
            continue;
        }
        
        memcpy(buffer, data, SIGNAL_LEN * sizeof(float));


        xQueueSend(raw_data_queue, &buffer, portMAX_DELAY);
    }
}
