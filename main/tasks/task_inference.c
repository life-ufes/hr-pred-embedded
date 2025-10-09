#include "tasks.h"
#include <stdbool.h>

// Just for tests
#define AGE        26
#define WEIGHT     78.0f      // kg
#define HEIGHT     1.80f      // m
#define MALE       true
#define FEMALE     (!MALE)

#define BMI        (WEIGHT / (HEIGHT * HEIGHT))


void task_inference(void *params){

    buffer_t * bf = NULL;

    while(1) {
        xQueueReceive(filtered_data_queue, &bf, portMAX_DELAY);

        printf("INFERENCE - Activity Level = %f\n", bf->al);

        xQueueSend(buffer_pool_queue, &bf, portMAX_DELAY);
    }
}
