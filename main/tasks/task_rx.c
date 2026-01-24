#include "tasks.h"
#include "driver/uart.h"

#define TEMP_BUFF_SIZE 512
#define PACKET_SIZE SERIAL_SIGNAL_LEN + 2

void task_rx(void *params)
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
            if (xQueueReceive(buffer_pool_queue, &buffer, pdMS_TO_TICKS(50)) == pdTRUE)
            {
                uart_read_bytes(uart_port_num, temp_buff, packet_size, pdMS_TO_TICKS(10));
                memcpy(signal_serial_window, temp_buff, packet_size);

                // Getting an available buffer
                buffer->hr_gt = signal_serial_window[SERIAL_SIGNAL_LEN];
                buffer->train = signal_serial_window[SERIAL_SIGNAL_LEN + 1];

                for (int i = 0; i < 3; i++)
                {
                    memcpy(buffer->acc[i], &signal_serial_window[i * 25], 25 * sizeof(float));
                }

                xQueueSend(raw_data_queue, &buffer, portMAX_DELAY);
            }
            else
            {
                // Discard packages to avoid uart buffer overflow when data comes quickly
                uart_flush_input(uart_port_num);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}


#define PKT_TYPE_DATA 0x01
#define PKT_TYPE_LOG  0x02

void send_deadbeef_packet(uint8_t type, const uint8_t *payload, uint16_t len) {
    const uint8_t header[] = {0xDE, 0xAD, 0xBE, 0xEF};
    uint8_t checksum = 0;

    // 1. Enviar o Header (4 bytes)
    uart_write_bytes(UART_NUM_0, (const char *)header, 4);

    // 2. Enviar o Tipo e o Tamanho (2 bytes para o tamanho permite pacotes maiores)
    uint8_t len_low = len & 0xFF;
    uint8_t len_high = (len >> 8) & 0xFF;
    
    uart_write_bytes(UART_NUM_0, (const char *)&type, 1);
    uart_write_bytes(UART_NUM_0, (const char *)&len_low, 1);
    uart_write_bytes(UART_NUM_0, (const char *)&len_high, 1);

    // Iniciar cálculo do Checksum com os metadados
    checksum ^= type;
    checksum ^= len_low;
    checksum ^= len_high;

    // 3. Enviar o Payload e calcular Checksum
    for (uint16_t i = 0; i < len; i++) {
        checksum ^= payload[i];
    }
    uart_write_bytes(UART_NUM_0, (const char *)payload, len);

    // 4. Enviar o Checksum final (1 byte)
    uart_write_bytes(UART_NUM_0, (const char *)&checksum, 1);
}