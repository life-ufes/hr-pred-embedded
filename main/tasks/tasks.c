#include "tasks.h"

// Global queues
QueueHandle_t buffer_pool_queue;
QueueHandle_t raw_data_queue;
QueueHandle_t filtered_data_queue;
QueueHandle_t inference_result_queue;

// Buffer pool
buffer_t buffer_p[NUM_BUFFERS];

void init_pipeline(void)
{
    buffer_pool_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t *));
    raw_data_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t *));
    filtered_data_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t *));
    inference_result_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t *));

    for (int x = 0; x < NUM_BUFFERS; x++)
    {
        buffer_t *bf_ptr = &buffer_p[x];
        xQueueSend(buffer_pool_queue, &bf_ptr, portMAX_DELAY);
    }
}
