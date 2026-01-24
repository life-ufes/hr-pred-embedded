#pragma once
#include <stdint.h>

#define PKT_TYPE_DATA 0x01
#define PKT_TYPE_LOG  0x02

typedef struct {
    float hr_gt;
    float al_raw;
    float al;
    float hr;
} __attribute__((packed)) telemetry_t;

void comm_init(void);
void comm_send_packet(uint8_t type, const uint8_t *payload, uint16_t len);