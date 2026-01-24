#include "comm_protocol.h"
#include "driver/uart.h"
#include "esp_log.h"

#define UART_NUM UART_NUM_0

static SemaphoreHandle_t uart_mutex = NULL;
static QueueHandle_t uart_event_queue; // to rx task

void comm_init(void);
void comm_send_packet(uint8_t type, const uint8_t *payload, uint16_t len);
QueueHandle_t comm_get_uart_queue(void);

/// @brief
/// @param
void comm_init(void)
{
    // UART TX Mutex
    if (uart_mutex == NULL)
    {
        uart_mutex = xSemaphoreCreateMutex();
    }

    // UART
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

/// @brief
/// @param type
/// @param payload
/// @param len
void comm_send_packet(uint8_t type, const uint8_t *payload, uint16_t len)
{
    if (uart_mutex == NULL)
        return;

    // Buffer temporário: 4(hdr) + 1(type) + 2(len) + len(payload) + 1(chk)
    // Para telemetria (16 bytes) + overhead = 24 bytes. 256 é seguro.
    uint8_t tx_buf[256];
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

        // UMA única chamada garante que o pacote saia inteiro
        uart_write_bytes(UART_NUM, (const char *)tx_buf, 7 + len + 1);

        xSemaphoreGive(uart_mutex);
    }
}

/// @brief
/// @param
/// @return
QueueHandle_t comm_get_uart_queue(void)
{
    return uart_event_queue;
}
