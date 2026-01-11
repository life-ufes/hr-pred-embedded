#include <stdio.h>
#include <stdlib.h>
#include "tasks.h"

// Just for tests
#define AGE        26
#define WEIGHT     78.0f      // kg
#define HEIGHT     1.80f      // m
#define MALE       true
#define FEMALE     (!MALE)
#define BMI        (WEIGHT / (HEIGHT * HEIGHT))

void app_main(void)
{
    init_uart();
    uart_mutex = xSemaphoreCreateMutex();

    init_pipeline();
    
    xTaskCreate(task_receive, "receive_task", 4096, NULL, 5, NULL);
    xTaskCreate(task_preprocess, "preprocess_task", 2048, NULL, 5, NULL);
    
    #ifdef CONFIG_EXPONENTIAL_APPROXIMATION_MODEL
        xTaskCreate(task_inference_eam, "inference_task", 4096, NULL, 5, NULL);
    #else
        xTaskCreate(task_inference_dem, "inference_task", 4096, NULL, 5, NULL);   
    #endif
    
    xTaskCreate(task_send, "send_task", 2048, NULL, 5, NULL);
}
