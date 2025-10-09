#pragma once

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"
#include "sigproc.h"

#define SIGNAL_LEN 25
#define NUM_BUFFERS 4


// Global queues
extern QueueHandle_t buffer_pool_queue;
extern QueueHandle_t raw_data_queue;
extern QueueHandle_t filtered_data_queue;
extern QueueHandle_t inference_result_queue;


// float buffers
extern float buffer_pool[NUM_BUFFERS][SIGNAL_LEN];

// union buffers
typedef union {
    float acc[SIGNAL_LEN * 3];
    float al;
    int hr;
} buffer_t;

extern buffer_t buffer_p[NUM_BUFFERS];


// Tasks declarations
void task_receive(void *params);
void task_preprocess(void *params);
void task_inference(void *params);
void task_send(void *params);

void init_pipeline(void);


// Utils
void print_buffer(float * buffer);
