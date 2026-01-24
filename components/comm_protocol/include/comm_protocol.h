#pragma once
#include <stdint.h>

#define PKT_TYPE_DATA 0x01
#define PKT_TYPE_LOG 0x02

/// @brief Telemetry data structure
typedef struct
{
    float hr;
    float hr_reg;
    float hr_gt;
    float al;
    float al_raw;
} __attribute__((packed)) telemetry_t;

/// @brief Initialize communication protocol
/// @param uart_port: UART port number to initialize communication on
void comm_init(void);

/// @brief Send a packet over UART
/// @param type: packet type (e.g., PKT_TYPE_DATA, PKT_TYPE_LOG)
/// @param payload: pointer to payload data (byte array)
/// @param len: length of payload data
void comm_send_packet(uint8_t type, const uint8_t *payload, uint16_t len);
