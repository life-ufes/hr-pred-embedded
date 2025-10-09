#include "tasks.h"

QueueHandle_t buffer_pool_queue;
QueueHandle_t raw_data_queue;
QueueHandle_t filtered_data_queue;
QueueHandle_t inference_result_queue;

// float buffers
// float buffer_pool[NUM_BUFFERS][SIGNAL_LEN];

buffer_t buffer_p[NUM_BUFFERS];

void init_pipeline(void) {
    // buffer_pool_queue = xQueueCreate(NUM_BUFFERS, sizeof(float*));
    // raw_data_queue = xQueueCreate(NUM_BUFFERS, sizeof(float*));
    // filtered_data_queue = xQueueCreate(NUM_BUFFERS, sizeof(float*));
    // inference_result_queue = xQueueCreate(NUM_BUFFERS, sizeof(float*));

    buffer_pool_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t*));
    raw_data_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t*));
    filtered_data_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t*));
    inference_result_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t*));

    // Insert buffers to pool
    // for(int x=0; x<NUM_BUFFERS; x++) {
        // float * buf_ptr = buffer_pool[x];
        // xQueueSend(buffer_pool_queue, &buf_ptr, portMAX_DELAY);
    // }

    for(int x=0; x<NUM_BUFFERS; x++) {
        buffer_t * bf_ptr = &buffer_p[x];
        xQueueSend(buffer_pool_queue, &bf_ptr, portMAX_DELAY);
    }
}

//  Utils
void print_buffer(float *buffer)
{
    printf("BUFFER: ");
    for(int x=0; x<SIGNAL_LEN; x++){
        printf("%f ", buffer[x]);
    }
    printf("\n");
}
