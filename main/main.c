#include <stdio.h>
#include <stdlib.h>
#include "freertos/queue.h"
#include "driver/uart.h"
#include "tasks/tasks.h"
#include "freertos/FreeRTOS.h"

QueueHandle_t queue_raw_data;
QueueHandle_t queue_filtered_data;
QueueHandle_t queue_inference_result;

void app_main(void)
{
    // xTaskCreatePinnedToCore()
}
