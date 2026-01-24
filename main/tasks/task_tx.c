#include "tasks.h"
#include "comm_protocol.h"

void task_tx(void *params)
{
    buffer_t *bf = NULL;

    while (1)
    {
        if (xQueueReceive(inference_result_queue, &bf, portMAX_DELAY) == pdTRUE)
        {
            // Mounts new packet
            telemetry_t data = {
                .hr = bf->hr,
                .hr_reg = bf->hr_reg,
                .hr_gt = bf->hr_gt,
                .al = bf->al,
                .al_raw = bf->al_raw,
            };

            comm_send_packet(PKT_TYPE_DATA, (uint8_t *)&data, sizeof(telemetry_t));
            xQueueSend(buffer_pool_queue, &bf, portMAX_DELAY);
        }
        else
        {
            // TODO: log
        }
    }
}
