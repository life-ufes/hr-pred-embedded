#include <stdio.h>
#include <stdlib.h>
#include "tasks.h"

void app_main(void)
{
    init_pipeline();

    xTaskCreate(task_receive, "receive_task", 4096, NULL, 5, NULL);
    xTaskCreate(task_preprocess, "preprocess_task", 2048, NULL, 5, NULL);
    xTaskCreate(task_inference, "inference_task", 2048, NULL, 5, NULL);
}
