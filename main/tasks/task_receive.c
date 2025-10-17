#include "tasks.h"
#include "driver/uart.h"

#define TEMP_BUFF_SIZE 512

static const char * TAG = "INPUT_TASK";

void task_receive(void *params)
{
    buffer_t *buffer;

    const int uart_baud_rate = 115200;
    const int uart_buffer_size = 1024;
    const int intr_alloc_flags = 0;
    const int queue_size = 5;
    const int uart_port_num = UART_NUM_0;
    QueueHandle_t uart_queue;

    uart_config_t uart_config = {
        .baud_rate = uart_baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT};

    ESP_ERROR_CHECK(uart_driver_install(uart_port_num, uart_buffer_size, uart_buffer_size, queue_size, &uart_queue, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(uart_port_num, &uart_config));

    float signal_serial_window[SERIAL_SIGNAL_LEN];
    uint8_t temp_buff[TEMP_BUFF_SIZE];
    size_t bytes_in_buff = 0;

    while (1)
    {
        ESP_ERROR_CHECK(uart_get_buffered_data_len(uart_port_num, &bytes_in_buff));

        if (bytes_in_buff >= SERIAL_SIGNAL_LEN * sizeof(float))
        {
            uart_read_bytes(uart_port_num, temp_buff, (SERIAL_SIGNAL_LEN * sizeof(float)), pdMS_TO_TICKS(0));
            memcpy(signal_serial_window, temp_buff, SERIAL_SIGNAL_LEN * sizeof(float));

            xQueueReceive(buffer_pool_queue, &buffer, portMAX_DELAY); // Catching an available buffer
            
            // Verfy buffer
            if (!buffer)
            {
                ESP_LOGW("receive_buffer", "NULL buffer");
                continue;
            }

            for (int i = 0; i < 3; i++)
            {
                memcpy(buffer->acc[i], &signal_serial_window[i * 25], 25 * sizeof(float));
            }

            xQueueSend(raw_data_queue, &buffer, portMAX_DELAY);

        } else {
            // ESP_LOGD(TAG, "");
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
