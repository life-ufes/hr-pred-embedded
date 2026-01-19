#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "sigproc.h"
#include "esp_dsp.h"

float acc_aggregation(const acc_t *const acc_sig)
{
    if(!acc_sig) {
        return 0.0f;
    }

    return sqrtf(
        (acc_sig->x * acc_sig->x) +
        (acc_sig->y * acc_sig->y) +
        (acc_sig->z * acc_sig->z)
    );
}

float ewma_update(ewma_t *const filter, const float input)
{
    filter->last_value = filter->alpha * input + (1 - filter->alpha) * filter->last_value;
    return filter->last_value;
}

void clip(float *agg_signal, int len, float cut)
{
    for (int i = 0; i < len; i++)
    {
        if (agg_signal[i] > cut)
        {
            agg_signal[i] = cut;
        }
    }
}

float trapz_integral(trapz_ctx_t *const ctx, const float *signal, const int len)
{
    float total_area = 0.0f;
    float current_value = 0.0f;
    float prev_value = ctx->prev;

    for (int i = 0; i < len; i++)
    {
        current_value = signal[i];

        // area of ​​the trapezoid
        float step_area = (prev_value + current_value) * (ctx->dt / 2.0f);
        total_area += step_area;

        prev_value = current_value;
    }

    ctx->prev = prev_value;

    return total_area;
}