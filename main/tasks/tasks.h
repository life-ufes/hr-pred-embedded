#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"


// Filas globais
extern QueueHandle_t queue_raw_data;
extern QueueHandle_t queue_filtered_data;
extern QueueHandle_t queue_inference_result;

// Declarações das tasks
void task_receive(void *params);
void task_preprocess(void *params);
void task_inference(void *params);
void task_send(void *params);
