#pragma once

#include <stdint.h>
#include "esp_err.h"

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
void comm_init();

/// @brief Send a packet over UART
/// @param type: packet type (e.g., PKT_TYPE_DATA, PKT_TYPE_LOG)
/// @param payload: pointer to payload data (byte array)
/// @param len: length of payload data
void comm_send_packet(uint8_t type, const uint8_t *payload, uint16_t len);

/// @brief Receive a packet over UART
/// @param out_type: pointer to packet type (e.g., PKT_TYPE_DATA, PKT_TYPE_LOG)
/// @param out_payload: pointer to buffer for payload data
/// @param out_len: pointer to variable to store length of received payload
/// @param max_len: maximum length of the output payload buffer
/// @return ESP_OK on success, ESP_FAIL on error
esp_err_t comm_receive_packet(uint8_t *out_type, uint8_t *out_payload, uint16_t *out_len, uint16_t max_len);
