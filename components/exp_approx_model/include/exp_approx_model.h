#pragma once

#define WEIGHTS_LEN 5

typedef enum {
    HIGH,
    LOW
} Intensity;

typedef enum {
    MALE,
    FEMALE
} Genre;

typedef struct exp_approx_model
{
    float ds[WEIGHTS_LEN];
    float weights[WEIGHTS_LEN];
    float hr_reg;
    float next_hr;
    float tau;
    float next_tau;
    float alpha;
    float b_low;
    float b_high;
    Intensity intensity;
} eam_t;


void ea_model_init(eam_t *model);

void ea_model_set_user_data(eam_t *model, float bmi, Genre g, int age);

void ea_model_handle_intensity(eam_t *model);

void ea_model_partial_fit(eam_t *model, float al, float hr_gt);

void ea_model_predict(eam_t *model);

void ea_model_debug(eam_t *model);
