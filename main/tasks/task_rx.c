#include "tasks.h"
#include "comm_protocol.h"

void task_rx(void *params)
{
    buffer_t *buffer = NULL;
    uint8_t p_type;
    uint16_t p_len;
    
    // temporary buffer for received payload
    static float rx_payload[SERIAL_SIGNAL_LEN + 2]; 

    while (1)
    {
        // Wait for a packet from the communication protocol
        if (comm_receive_packet(&p_type, (uint8_t*)rx_payload, &p_len, sizeof(rx_payload)) == ESP_OK)
        {
            if (p_type == PKT_TYPE_DATA)
            {
                // Get a buffer from the pool
                if (xQueueReceive(buffer_pool_queue, &buffer, pdMS_TO_TICKS(50)) == pdTRUE)
                {
                    // Fill the buffer with received data
                    buffer->hr_gt = rx_payload[SERIAL_SIGNAL_LEN];
                    buffer->train = rx_payload[SERIAL_SIGNAL_LEN + 1];

                    for (int i = 0; i < 3; i++) {
                        memcpy(buffer->acc[i], &rx_payload[i * 25], 25 * sizeof(float));
                    }

                    // Send the filled buffer to the next stage
                    xQueueSend(raw_data_queue, &buffer, portMAX_DELAY);
                }
            }
        }
    }
}
