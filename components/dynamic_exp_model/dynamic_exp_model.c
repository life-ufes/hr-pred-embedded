#include <stdio.h>
#include "dynamic_exp_model.h"

// TODO: change weights based on pre config menu and pre-process directives
#define INITIAL_WEIGHT_1 93.75f
#define INITIAL_WEIGHT_2 0.82f
#define INITIAL_WEIGHT_3 -0.90f
#define INITIAL_WEIGHT_4 -6.44f
#define INITIAL_WEIGHT_5 -0.09f
#define INITIAL_WEIGHT_6 -0.09f

#define TAU_RISE 56.25f
#define TAU_DECAY 56.25f

#define B_HIGH 60.0f
#define B_LOW 40.0f

#define L_RATE 0.0092f
#define INTENSITY_TRESHOLD 1.0f

void de_model_init(dem_t *model)
{
	float weights[WEIGHTS_LEN] = {
		INITIAL_WEIGHT_1,
		INITIAL_WEIGHT_2,
		INITIAL_WEIGHT_3,
		INITIAL_WEIGHT_4,
		INITIAL_WEIGHT_5
		INITIAL_WEIGHT_6};

	memcpy(model->weights, weights, WEIGHTS_LEN * sizeof(float));

	model->tau_rise = TAU_RISE;
	model->tau_decay = TAU_DECAY;
	model->next_tau = TAU_RISE;

	model->b_low = B_LOW;
	model->b_high = B_HIGH;
	
	model->hr_reg = 85.0f;
	model->next_hr = 85.0f;
	
	model->alpha = L_RATE;
	model->intensity = LOW;

	float ds[WEIGHTS_LEN] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	memcpy(model->ds, ds, WEIGHTS_LEN * sizeof(float));
}


void de_model_set_user_data(dem_t *model, float bmi, gender_dem_t g, int age)
{
	model->ds[3] = bmi;
	model->ds[4] = (float)age;
	model->ds[5] = g == MALE ? 1.0f : 0.0f;
}


void de_model_handle_intensity(dem_t *model)
{
	model->intensity = model->ds[0] >= INTENSITY_TRESHOLD ? HIGH : LOW;
}


void de_model_set_als(dem_t *model, float al[3])
{
	model->ds[0] = al[0];
	model->ds[1] = al[1];
	model->ds[2] = al[2];
}
