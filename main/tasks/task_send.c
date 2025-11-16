#include "tasks.h"

void task_send(void *params)
{
    buffer_t * bf = NULL;
    const int uart_port = UART_NUM_0;

    while (1)
    {
        xQueueReceive(inference_result_queue, &bf, portMAX_DELAY);

        xSemaphoreTake(uart_mutex, portMAX_DELAY);
        uart_write_bytes(uart_port, &bf->hr, sizeof(int));
        // printf("INFERENCE - HR = %d\n", bf->hr);
        xSemaphoreGive(uart_mutex);

        xQueueSend(buffer_pool_queue, &bf, portMAX_DELAY);
    }
}
