#include "tasks.h"
#include "comm_protocol.h"

void task_tx(void *params)
{
    buffer_t *bf = NULL;
    const int uart_port = UART_NUM_0;

    while (1)
    {
        xQueueReceive(inference_result_queue, &bf, portMAX_DELAY);

        // xSemaphoreTake(uart_mutex, portMAX_DELAY);
        // uart_write_bytes(uart_port, &bf->hr, sizeof(float));

        // printf("INFERENCE - HR = %d\n", bf->hr);
        // xSemaphoreGive(uart_mutex);
        telemetry_t data = {
            .hr = bf->hr,
            .hr_gt = bf->hr_gt,
            .al = bf->al_raw,
            .al_raw = bf->al_raw,
        };

        comm_send_packet(PKT_TYPE_DATA, (uint8_t *)&data, sizeof(telemetry_t));

        xQueueSend(buffer_pool_queue, &bf, portMAX_DELAY);
    }
}
