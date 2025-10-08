#pragma once

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "esp_err.h"

#define SIGNAL_LEN 25
#define NUM_BUFFERS 4


// Global queues
extern QueueHandle_t buffer_pool_queue;
extern QueueHandle_t raw_data_queue;
extern QueueHandle_t filtered_data_queue;
extern QueueHandle_t inference_result_queue;

// Buffers
extern float buffer_pool[NUM_BUFFERS][SIGNAL_LEN];

// Tasks declarations
void task_receive(void *params);
void task_preprocess(void *params);
void task_inference(void *params);
void task_send(void *params);

void init_pipeline(void);
