#include "tasks.h"
#include "comm_protocol.h"

void task_tx(void *params)
{
    buffer_t *bf = NULL;

    while (1)
    {
        // Receive buffer from the previous stage
        if (xQueueReceive(inference_result_queue, &bf, portMAX_DELAY) == pdTRUE)
        {
            // Mounts new packet
            telemetry_t data = {
                .hr = bf->hr,
                .hr_reg = bf->hr_reg,
                .hr_gt = bf->hr_gt,
                .al = bf->al,
                .al_raw = bf->al_raw,
                .pre_process_time = bf->pre_process_time,
                .pre_process_hwm = bf->pre_process_hwm,
                .inference_time = bf->inference_time,
                .inference_hwm = bf->inference_hwm,
            };

            // Send packet via communication protocol
            comm_send_packet(PKT_TYPE_DATA, (uint8_t *)&data, sizeof(telemetry_t));
            
            // Return buffer to pool
            xQueueSend(buffer_pool_queue, &bf, portMAX_DELAY);
        }
        else
        {
            // TODO: log
        }
    }
}
