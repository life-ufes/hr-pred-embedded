#include "tasks.h"
#include "driver/uart.h"

#define TEMP_BUFF_SIZE 512
#define PACKET_SIZE SERIAL_SIGNAL_LEN + 2

static const char * TAG = "INPUT_TASK";

void task_receive(void *params)
{
    const int uart_port_num = UART_NUM_0;
    float signal_serial_window[PACKET_SIZE];
    uint8_t temp_buff[TEMP_BUFF_SIZE];
    size_t bytes_in_buff = 0;
    buffer_t *buffer = NULL;

    const size_t packet_size = (PACKET_SIZE) * sizeof(float);

    while (1)
    {
        ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_port_num, &bytes_in_buff));

        if (bytes_in_buff >= packet_size)
        {
            uart_read_bytes(uart_port_num, temp_buff, packet_size, pdMS_TO_TICKS(0));
            memcpy(signal_serial_window, temp_buff, packet_size);

            // Catching an available buffer
            xQueueReceive(buffer_pool_queue, &buffer, portMAX_DELAY); 
            
            // Checks buffer
            if (!buffer)
            {
                ESP_LOGE("receive_buffer", "NULL buffer");
                continue;
            }
            
            // Fill buffer
            buffer->hr_gt = signal_serial_window[SERIAL_SIGNAL_LEN];
            buffer->train = signal_serial_window[SERIAL_SIGNAL_LEN + 1];

            for (int i = 0; i < 3; i++)
            {
                memcpy(buffer->acc[i], &signal_serial_window[i * 25], 25 * sizeof(float));
            }

            xQueueSend(raw_data_queue, &buffer, portMAX_DELAY);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
