#pragma once

void calc_accel_mag_vec(float *x, float *y, float *z, float *out, int len);

// Exponential Weighted Moving Average
typedef struct ewma_t
{
    float alpha;
    float last_value;
} ewma_t;

float ewma_update(ewma_t *const filter, const float input);

// Trapezoidal integral
typedef struct
{
    float prev;
    float dt;
} trapz_ctx_t;

float trapz_integral(trapz_ctx_t *const ctx, const float *signal, const int len);

// Cut off
void clip(float *agg_signal, int len, float cut);
