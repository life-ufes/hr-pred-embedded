#include "tasks.h"
#include "esp_dsp.h"

// Using 3 coeffs only. The 4th's used to memory alignment.
#define COEFFS_LEN 4

// Decl
void task_preprocess(void *params);
float activity_level(const float *const signal, int len);


// Impl
void task_preprocess(void *params){
    static __attribute__((aligned(16))) float fir_coeffs[COEFFS_LEN] = {0.05637224f, 0.9087878f, 0.05637224f, 0.0f};
    static __attribute__((aligned(16))) float delay_line[COEFFS_LEN + 4];
    static __attribute__((aligned(16))) float temp_input[SIGNAL_LEN] = {0};

    fir_f32_t fir;
    // float * buffer = NULL;
    float agg[SIGNAL_LEN]= {0};

    buffer_t * bf = NULL;


    // Initialize fir filter
    ESP_ERROR_CHECK(dsps_fir_init_f32(&fir, fir_coeffs, delay_line, COEFFS_LEN));
    ESP_LOGI("PREPROCESS", "Filter initialized successfully");

    // Task loop
    while(1) {
        // Receive raw data
        xQueueReceive(raw_data_queue, &bf, portMAX_DELAY);
        
        // Uses auxiliar buffer
        memcpy(temp_input, bf->acc, SIGNAL_LEN * sizeof(float));
        
        // Filtering
        dsps_fir_f32_aes3(&fir, temp_input, bf->acc, SIGNAL_LEN);    
        
        // Aggregating  -> Just simulating...
        // for(int x=0; x<SIGNAL_LEN; x++) {
        //     acc_t sig = { 
        //         .x = buffer[x], 
        //         .y = (buffer[x] + 1), 
        //         .z = (buffer[x] + 1)
        //     };
        //     buffer[x] = acc_aggregation(&sig);
        // }

        for(int x=0; x<SIGNAL_LEN; x++) {
            acc_t sig = { 
                .x = bf->acc[x * 3 + 0], 
                .y = bf->acc[x * 3 + 1], 
                .z = bf->acc[x * 3 + 2]
            };
            agg[x] = acc_aggregation(&sig);
        }

        // EWMA
        // buffer[0] = activity_level(buffer, SIGNAL_LEN);
        bf->al = activity_level(agg, SIGNAL_LEN);

        // Send data to the next stage
        xQueueSend(filtered_data_queue, &bf, portMAX_DELAY);    
    }
}


// Activity level computing
float activity_level(const float *const signal, int len)
{
    ewma_t ewma_filter = {
        .alpha = 0.0769,    // Calculated manually
        .last_value = signal[0]
    };
    for(int x=0; x<len; x++){
        ewma_update(&ewma_filter, signal[x]);
    }
    return ewma_filter.last_value;
}
