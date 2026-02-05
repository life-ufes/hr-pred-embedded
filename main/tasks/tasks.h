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
#define SENSOR_AXES 3
#define RAW_SIGNAL_CHANNELS (SENSOR_AXES * 2)
#define SERIAL_SIGNAL_LEN (WINDOW_LEN * RAW_SIGNAL_CHANNELS)
#define SERIAL_SIGNAL_LEN WINDOW_LEN * 3

// Global queues
extern QueueHandle_t buffer_pool_queue;
extern QueueHandle_t raw_data_queue;
extern QueueHandle_t filtered_data_queue;
extern QueueHandle_t inference_result_queue;

// Buffer structure
typedef struct
{
    float hr_gt; 
    int train;   
    float al_raw;
    float al;    

    // Aligned accelerometer and gyroscope data for DSP operations
    float acc[3][WINDOW_LEN_ALIGNMENT] __attribute__((aligned(16)));
    float gyro[3][WINDOW_LEN_ALIGNMENT] __attribute__((aligned(16)));
    float acc[3][WINDOW_LEN];
    float hr;
    float hr_reg;

    float pre_process_time;
    unsigned int pre_process_hwm;
    float inference_time;
    unsigned int inference_hwm;

} buffer_t;

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
