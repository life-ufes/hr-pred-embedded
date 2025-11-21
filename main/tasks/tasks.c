#include "tasks.h"

QueueHandle_t buffer_pool_queue;
QueueHandle_t raw_data_queue;
QueueHandle_t filtered_data_queue;
QueueHandle_t inference_result_queue;
buffer_t buffer_p[NUM_BUFFERS];
SemaphoreHandle_t uart_mutex;

void init_pipeline(void) {

    buffer_pool_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t*));
    raw_data_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t*));
    filtered_data_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t*));
    inference_result_queue = xQueueCreate(NUM_BUFFERS, sizeof(buffer_t*));

    for(int x=0; x<NUM_BUFFERS; x++) {
        buffer_t * bf_ptr = &buffer_p[x];
        xQueueSend(buffer_pool_queue, &bf_ptr, portMAX_DELAY);
    }
}

void init_uart(void)
{
    const int uart_baud_rate = 115200;
    const int uart_buffer_size = 1024;
    const int intr_alloc_flags = 0;
    const int queue_size = 5;
    const int uart_port_num = UART_NUM_0;
    QueueHandle_t uart_queue;

    uart_config_t uart_config = {
        .baud_rate = uart_baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT};

    ESP_ERROR_CHECK(uart_driver_install(uart_port_num, uart_buffer_size, uart_buffer_size, queue_size, &uart_queue, intr_alloc_flags));
    ESP_ERROR_CHECK(uart_param_config(uart_port_num, &uart_config));
}

//  Utils
void print_buffer(float *buffer)
{
    printf("BUFFER: ");
    for(int x=0; x<WINDOW_LEN; x++){
        printf("%f ", buffer[x]);
    }
    printf("\n");
}
