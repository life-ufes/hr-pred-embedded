#pragma once

// Signal aggregation
typedef struct acc_t
{
    float x;
    float y;
    float z;
} acc_t;

float acc_aggregation(const acc_t *const acc_sig);

// Exponential Weighted Moving Average
typedef struct ewma_t
{
    float alpha;
    float last_value;
} ewma_t;

float ewma_update(ewma_t *const filter, const float input);

typedef struct
{
    float prev;
    float dt;
} trapz_ctx_t;

// Trapezoidal integral
float trapz_integral(trapz_ctx_t *const ctx, const float *signal, const int len);

void clip(float *agg_signal, int len, float cut);
