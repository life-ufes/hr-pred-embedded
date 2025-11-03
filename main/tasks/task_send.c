#include "tasks.h"

void task_send(void *params)
{
    buffer_t * bf = NULL;
    while (1)
    {
        xQueueReceive(inference_result_queue, &bf, portMAX_DELAY);
        printf("INFERENCE - HR = %d\n", bf->hr);
        xQueueSend(buffer_pool_queue, &bf, portMAX_DELAY);
    }
}
