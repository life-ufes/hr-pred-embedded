#include "comm_protocol.h"
#include "driver/uart.h"
#include "esp_log.h"

// Implementação da send_deadbeef_packet que fizemos...
#define UART_NUM UART_NUM_0 // UART padrão para logs e dados

void comm_send_packet(uint8_t type, const uint8_t *payload, uint16_t len)
{
    // 1. Cabeçalho Mágico (Sync Word)
    const uint8_t header[] = {0xDE, 0xAD, 0xBE, 0xEF};

    // 2. Metadados
    uint8_t len_low = len & 0xFF;
    uint8_t len_high = (len >> 8) & 0xFF;

    // 3. Cálculo do Checksum (XOR)
    // Começamos o checksum com os metadados (tipo e tamanho)
    uint8_t checksum = type ^ len_low ^ len_high;

    // Acumulamos o payload no checksum
    for (uint16_t i = 0; i < len; i++)
    {
        checksum ^= payload[i];
    }

    // 4. Transmissão sequencial
    // Enviamos o header
    uart_write_bytes(UART_NUM, (const char *)header, 4);

    // Enviamos Tipo e Tamanho (3 bytes)
    uart_write_bytes(UART_NUM, (const char *)&type, 1);
    uart_write_bytes(UART_NUM, (const char *)&len_low, 1);
    uart_write_bytes(UART_NUM, (const char *)&len_high, 1);

    // Enviamos o Payload
    uart_write_bytes(UART_NUM, (const char *)payload, len);

    // Enviamos o Checksum (1 byte final)
    uart_write_bytes(UART_NUM, (const char *)&checksum, 1);
}

// Handler para redirecionar o ESP_LOG
static int comm_log_vprintf(const char *fmt, va_list l)
{
    char buf[128];
    int len = vsnprintf(buf, sizeof(buf), fmt, l);
    if (len > 0)
    {
        comm_send_packet(PKT_TYPE_LOG, (uint8_t *)buf, len);
    }
    return len;
}

void comm_init(void)
{
    // Configura a UART se necessário ou apenas o log hook
    esp_log_set_vprintf(comm_log_vprintf);
}