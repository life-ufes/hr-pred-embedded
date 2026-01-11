#pragma once

#define WEIGHTS_LEN 6

typedef enum {
  HIGH,
  LOW
} intensity_dem_t;

typedef enum {
  MALE,
  FEMALE
} gender_dem_t;

typedef enum {
  RISE,
  DECAY
} phase_dem_t;


typedef struct dynamic_exp_model
{
    float ds[WEIGHTS_LEN];
    float weights[WEIGHTS_LEN];
    
    float hr_reg;
    float hr;
    float next_hr;
    
    float tau_rise;
    float tau_decay;
    float tau;
    
    float b_low;
    float b_high;
    
    float alpha;
    intensity_dem_t intensity;
    phase_dem_t phase;

}__attribute__((aligned(16))) dem_t;


void de_model_init(dem_t *model);

void de_model_set_user_data(dem_t *model, float bmi, gender_dem_t g, int age);

void de_model_handle_intensity(dem_t *model);

void de_model_handle_phase(dem_t *model);

void de_model_partial_fit(dem_t *model, float hr_gt);

void de_model_predict(dem_t *model);

void de_model_debug(dem_t *model, float hr_gt);

void de_model_set_als(dem_t *model, float al[3]);
