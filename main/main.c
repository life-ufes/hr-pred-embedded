#include <stdio.h>
#include <stdlib.h>
#include "tasks.h"
#include "comm_protocol.h"

void app_main(void)
{
    comm_init();
    init_pipeline();
    
    #ifdef CONFIG_HARDWARE_ACCELERATION_ENABLED
        printf("Hardware acceleration enabled: Using optimized DSP functions.\n");
    #else
        printf("Hardware acceleration disabled: Using ANSI C implementations.\n");
    #endif

    xTaskCreate(task_rx, "task_rx", 4096, NULL, 5, NULL);
    xTaskCreate(task_preprocess, "preprocess_task", 6144, NULL, 5, NULL);

#ifdef CONFIG_EXPONENTIAL_APPROXIMATION_MODEL
    xTaskCreate(task_inference_eam, "inference_task", 1024, NULL, 5, NULL);
#else
    xTaskCreate(task_inference_dem, "inference_task", 4096, NULL, 5, NULL);
#endif

    xTaskCreate(task_tx, "task_tx", 2048, NULL, 5, NULL);
}
