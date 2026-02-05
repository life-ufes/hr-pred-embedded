#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "sigproc.h"
#include "esp_dsp.h"

// ----------------------------------------------------------------------
void calc_accel_mag_vec(float *x, float *y, float *z, float *out, int len)
{
    for (int i = 0; i < len; i++) {
        float xx = x[i] * x[i];
        float yy = y[i] * y[i];
        float zz = z[i] * z[i];

        out[i] = sqrtf(xx + yy + zz);
    }
}

// ------------------------------------------------------
float ewma_update(ewma_t *const filter, const float input)
{
    filter->last_value = filter->alpha * input + (1 - filter->alpha) * filter->last_value;
    return filter->last_value;
}

// --------------------------------------------
void clip(float *agg_signal, int len, float cut)
{
    for (int i = 0; i < len; i++)
    {
        agg_signal[i] = fminf(agg_signal[i], cut);
    }
}

// -----------------------------------------------------------------------------
float trapz_integral(trapz_ctx_t *const ctx, const float *signal, const int len)
{
    float running_sum = 0.0f;
    float prev_value = ctx->prev;

    for (int i = 0; i < len; i++)
    {
        running_sum += (prev_value + signal[i]);
        prev_value = signal[i];
    }

    ctx->prev = prev_value;
    return running_sum * (ctx->dt * 0.5f);
}
