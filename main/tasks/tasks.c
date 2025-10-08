#include "tasks.h"

QueueHandle_t buffer_pool_queue;
QueueHandle_t raw_data_queue;
QueueHandle_t filtered_data_queue;
QueueHandle_t inference_result_queue;

float buffer_pool[NUM_BUFFERS][SIGNAL_LEN];

void init_pipeline(void) {
    buffer_pool_queue = xQueueCreate(NUM_BUFFERS, sizeof(float*));
    
    raw_data_queue = xQueueCreate(NUM_BUFFERS, sizeof(float*));
    filtered_data_queue = xQueueCreate(NUM_BUFFERS, sizeof(float*));
    inference_result_queue = xQueueCreate(NUM_BUFFERS, sizeof(float*));

    // Insert buffers to pool
    for(int x=0; x<NUM_BUFFERS; x++) {
        float * buf_ptr = buffer_pool[x];
        xQueueSend(buffer_pool_queue, &buf_ptr, portMAX_DELAY);
    }
}
