#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "sigproc.h"
#include "esp_dsp.h"

float acc_aggregation(const acc_t *const acc_sig)
{
    return sqrtf(powf(acc_sig->x, 2) + powf(acc_sig->y, 2) + powf(acc_sig->z, 2));
}


float ewma_update(ewma_t *const filter, const float input)
{
    filter->last_value = filter->alpha * input + (1 - filter->alpha) * filter->last_value;
    return filter->last_value;
}
