#include "tasks.h"
#include "esp_dsp.h"

// Using 3 coeffs only. The 4th's used to memory alignment.
#define COEFFS_LEN 4


void task_preprocess(void *params){
    static __attribute__((aligned(16))) float fir_coeffs[COEFFS_LEN] = {0.05637224f, 0.9087878f, 0.05637224f, 0.0f};
    static __attribute__((aligned(16))) float delay_line[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) float temp_input[SIGNAL_LEN] = {0};

    fir_f32_t fir;

    float * buffer = NULL;

    // Initialize fir filter
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir, fir_coeffs, delay_line, COEFFS_LEN));
    ESP_LOGI("PREPROCESS", "Filter initialized successfully");

    while(1) {
        xQueueReceive(raw_data_queue, &buffer, portMAX_DELAY);      // Waits until an item arrives
        memcpy(temp_input, buffer, SIGNAL_LEN * sizeof(float));
        
        dsps_fir_f32_aes3(&fir, temp_input, buffer, SIGNAL_LEN);    // Filtering    
        xQueueSend(filtered_data_queue, &buffer, portMAX_DELAY);    
    }
}
