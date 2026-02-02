#pragma once

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/uart.h"

#define NUM_BUFFERS 4
#define WINDOW_LEN 25
#define WINDOW_LEN_ALIGNMENT 28
#define SERIAL_SIGNAL_LEN WINDOW_LEN * 3

// Global queues
extern QueueHandle_t buffer_pool_queue;
extern QueueHandle_t raw_data_queue;
extern QueueHandle_t filtered_data_queue;
extern QueueHandle_t inference_result_queue;

// Buffer structure
typedef struct
{
    float hr_gt;  // 0 to 4 bytes
    int train;    // 4 to 8 bytes
    float al_raw; // 8 to 12 bytes
    float al;     // 12 to 16 bytes

    // Aligned accelerometer data for DSP operations (112 bytes per axis)
    float acc[3][WINDOW_LEN_ALIGNMENT] __attribute__((aligned(16)));
    float hr;
    float hr_reg;

    float pre_process_time;
    unsigned int pre_process_hwm;
    float inference_time;
    unsigned int inference_hwm;

} __attribute__((aligned(16))) buffer_t;

// Buffer pool
extern buffer_t buffer_p[NUM_BUFFERS];

// Tasks decl
void task_rx(void *params);
void task_tx(void *params);
void task_preprocess(void *params);
void task_inference_eam(void *params);
void task_inference_dem(void *params);

// Pipeline init
void init_pipeline(void);
