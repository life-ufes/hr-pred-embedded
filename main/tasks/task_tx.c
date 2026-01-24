#include "tasks.h"
#include "comm_protocol.h"

void task_tx(void *params)
{
    buffer_t *bf = NULL;
    const int uart_port = UART_NUM_0;
    telemetry_t data;

    while (1)
    {
        xQueueReceive(inference_result_queue, &bf, portMAX_DELAY);

        // xSemaphoreTake(uart_mutex, portMAX_DELAY);
        // uart_write_bytes(uart_port, &bf->hr, sizeof(float));

        // printf("INFERENCE - HR = %d\n", bf->hr);
        // xSemaphoreGive(uart_mutex);
        data.hr_gt = bf->hr_gt;
        data.hr = bf->hr;
        data.al_raw = bf->al_raw;
        data.al = bf->al_raw;

        // TODO: create uart mutex within comm_protocol component
        // TODO: Initialize uart within comm_protocol
        comm_send_packet(PKT_TYPE_DATA, (uint8_t *)&data, sizeof(telemetry_t));

        xQueueSend(buffer_pool_queue, &bf, portMAX_DELAY);
    }
}
