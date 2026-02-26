#include "tasks.h"
#include "comm_protocol.h"

void task_rx(void *params)
{
    buffer_t *buffer = NULL;
    uint8_t p_type;
    uint16_t p_len;
    
    static rx_payload_t rx_payload; 

    while (1)
    {
        if (comm_receive_packet(&p_type, (uint8_t*)&rx_payload, &p_len, sizeof(rx_payload)) == ESP_OK)
        {
            if (p_type == PKT_TYPE_DATA)
            {
                if (xQueueReceive(buffer_pool_queue, &buffer, pdMS_TO_TICKS(50)) == pdTRUE)
                {
                    buffer->hr_gt = rx_payload.hr_gt;
                    buffer->train = rx_payload.train;

                    for (int i = 0; i < 3; i++) {
                        memcpy(buffer->raw_acc[i], rx_payload.acc_raw[i], 25 * sizeof(int16_t));
                    }

                    xQueueSend(raw_data_queue, &buffer, portMAX_DELAY);
                }
            }
        }
    }
}
