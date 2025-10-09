#pragma once

#define W_LEN 5

typedef struct exp_approx_model
{
    float ds[W_LEN];
    float weights[W_LEN];
    float hr_til;
    float hr;
    float tau;
    float alpha;
    float b_low;
    float b_high;
} eam_t;


void ea_model_init();

void ea_model_fit(eam_t *model);

int ea_model_predict(eam_t *model);
