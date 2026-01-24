#include "comm_protocol.h"
#include "driver/uart.h"
#include "esp_log.h"

#define UART_NUM UART_NUM_0

static SemaphoreHandle_t uart_mutex = NULL;
static QueueHandle_t uart_event_queue;

// -------------
void comm_init()
{
    // Create TX mutex
    if (uart_mutex == NULL)
    {
        uart_mutex = xSemaphoreCreateMutex();
    }

    // init UART
    const int uart_port_num = UART_NUM_0;
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT};

    // Driver install
    ESP_ERROR_CHECK(uart_driver_install(uart_port_num, 1024, 1024, 5, &uart_event_queue, 0));
    ESP_ERROR_CHECK(uart_param_config(uart_port_num, &uart_config));
}

// ----------------------------------------------------------------------
void comm_send_packet(uint8_t type, const uint8_t *payload, uint16_t len)
{
    if (uart_mutex == NULL)
        return;

    // Temp buffer: 4(hdr) + 1(type) + 2(len) + len(payload) + 1(chk)
    uint8_t tx_buf[64];
    if (len > (256 - 8))
        return;

    if (xSemaphoreTake(uart_mutex, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        tx_buf[0] = 0xDE;
        tx_buf[1] = 0xAD;
        tx_buf[2] = 0xBE;
        tx_buf[3] = 0xEF;
        tx_buf[4] = type;
        tx_buf[5] = len & 0xFF;
        tx_buf[6] = (len >> 8) & 0xFF;

        uint8_t checksum = tx_buf[4] ^ tx_buf[5] ^ tx_buf[6];
        for (uint16_t i = 0; i < len; i++)
        {
            tx_buf[7 + i] = payload[i];
            checksum ^= payload[i];
        }
        tx_buf[7 + len] = checksum;

        uart_write_bytes(UART_NUM, (const char *)tx_buf, 7 + len + 1);
        xSemaphoreGive(uart_mutex);
    }
}

// -------------------------------------------------------------------------------------------------------
esp_err_t comm_receive_packet(uint8_t *out_type, uint8_t *out_payload, uint16_t *out_len, uint16_t max_len)
{
    uart_event_t event;
    uint8_t byte;

    // Wait for UART event
    if (xQueueReceive(uart_event_queue, &event, portMAX_DELAY))
    {
        if (event.type == UART_DATA)
        {
            // Find header
            while (uart_read_bytes(UART_NUM, &byte, 1, pdMS_TO_TICKS(10)) > 0)
            {
                if (byte == 0xDE)
                {
                    uint8_t head[3];
                    if (uart_read_bytes(UART_NUM, head, 3, pdMS_TO_TICKS(20)) == 3)
                    {
                        if (head[0] == 0xAD && head[1] == 0xBE && head[2] == 0xEF)
                        {
                            // packet metadata
                            uint8_t meta[3];
                            if (uart_read_bytes(UART_NUM, meta, 3, pdMS_TO_TICKS(20)) == 3)
                            {
                                *out_type = meta[0];
                                *out_len = meta[1] | (meta[2] << 8);

                                if (*out_len > max_len)
                                    return ESP_ERR_INVALID_SIZE;

                                // Payload (wait up to 100ms)
                                if (uart_read_bytes(UART_NUM, out_payload, *out_len, pdMS_TO_TICKS(100)) == *out_len)
                                {
                                    uint8_t rx_chk;
                                    uart_read_bytes(UART_NUM, &rx_chk, 1, pdMS_TO_TICKS(20));

                                    // Checksum XOR
                                    uint8_t calc_chk = meta[0] ^ meta[1] ^ meta[2];
                                    for (int i = 0; i < *out_len; i++)
                                        calc_chk ^= out_payload[i];

                                    if (calc_chk == rx_chk)
                                        return ESP_OK;
                                    return ESP_ERR_INVALID_CRC;
                                }
                            }
                        }
                    }
                }
            }
        }
        else if (event.type == UART_FIFO_OVF || event.type == UART_BUFFER_FULL)
        {
            // Clear UART buffer
            uart_flush_input(UART_NUM);
            return ESP_ERR_INVALID_STATE;
        }
    }
    return ESP_FAIL;
}
