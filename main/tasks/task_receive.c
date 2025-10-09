#include "tasks.h"

void task_receive(void *params)
{
    buffer_t * buffer;

    // Simulating data receive
    float data[3][SIGNAL_LEN] = {0};

    for(int x=0; x<3; x++) {
        for (int y = 0; y < SIGNAL_LEN; y++)
        {
            data[x][y] = y;
        }
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
        
        for (int i = 0; i < 3; i++)
        {
            memcpy(buffer->acc[i], data[i], SIGNAL_LEN * sizeof(float));
        }

        xQueueSend(raw_data_queue, &buffer, portMAX_DELAY);
    }
}
