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
    init_pipeline();

    xTaskCreate(task_receive, "receive_task", 4096, NULL, 5, NULL);
    xTaskCreate(task_preprocess, "preprocess_task", 2048, NULL, 5, NULL);
    xTaskCreate(task_inference, "inference_task", 2048, NULL, 5, NULL);
    xTaskCreate(task_send, "send_task", 2048, NULL, 5, NULL);
}
