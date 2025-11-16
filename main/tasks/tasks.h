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
#define SERIAL_SIGNAL_LEN WINDOW_LEN * 3

// Global queues
extern QueueHandle_t buffer_pool_queue;
extern QueueHandle_t raw_data_queue;
extern QueueHandle_t filtered_data_queue;
extern QueueHandle_t inference_result_queue;

extern SemaphoreHandle_t uart_mutex;

// union buffers
typedef struct {
    float hr_gt;
    
    union {
        float acc[3][WINDOW_LEN];
        float al;
        int hr;
    };
} buffer_t;

extern buffer_t buffer_p[NUM_BUFFERS];

// Tasks declarations
void task_receive(void *params);
void task_preprocess(void *params);
void task_inference(void *params);
void task_send(void *params);

void init_pipeline(void);
void init_uart(void);


// Utils
void print_buffer(float * buffer);
