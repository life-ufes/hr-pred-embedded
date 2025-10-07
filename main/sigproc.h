#pragma once

// Signal aggregation
typedef struct acc_t {
    float x;
    float y;
    float z;
} acc_t;

float acc_aggregation(const acc_t * const acc_sig);


// Exponential Weighted Moving Average
typedef struct ewma_t {
    float alpha;
    float last_value;
} ewma_t;


float ewma_update(ewma_t * const filter, const float input);


// FIR
void generate_bandpass_FIR_coefficients(float *fir_coeffs, const unsigned int fir_len, const float ft1, const float ft2);

