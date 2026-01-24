#include <math.h>
#include <stdio.h>
#include "esp_dsp.h"
#include "dynamic_exp_model.h"
#include "dem_weights.h"


// -----------------------------
void de_model_init(dem_t *model)
{
	float weights[WEIGHTS_LEN] = {
		INITIAL_WEIGHT_1,
		INITIAL_WEIGHT_2,
		INITIAL_WEIGHT_3,
		INITIAL_WEIGHT_4,
		INITIAL_WEIGHT_5,
		INITIAL_WEIGHT_6};

	memcpy(model->weights, weights, WEIGHTS_LEN * sizeof(float));

	model->tau_rise = TAU_RISE;
	model->tau_decay = TAU_DECAY;
	model->tau = TAU_RISE;

	model->b_low = B_LOW;
	model->b_high = B_HIGH;

	model->hr_reg = 85.0f;
	model->next_hr = 85.0f;

	model->alpha = L_RATE;
	model->intensity = LOW;

	float ds[WEIGHTS_LEN] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	memcpy(model->ds, ds, WEIGHTS_LEN * sizeof(float));
}

// --------------------------------------------------------------------------
void de_model_set_user_data(dem_t *model, float bmi, gender_dem_t g, int age)
{
	model->ds[3] = bmi;
	model->ds[4] = (float)age;
	model->ds[5] = g == MALE ? 1.0f : 0.0f;
}

// -----------------------------------------
void de_model_handle_intensity(dem_t *model , float al_raw)
{
	static int debounce_counter = 0;
    
    intensity_dem_t detected_intensity = (al_raw >= INTENSITY_THRESHOLD) ? HIGH : LOW;

    if (detected_intensity != model->intensity) {
        debounce_counter++;

        if (debounce_counter >= INTENSITY_DEBOUNCE_LIMIT) {
            model->intensity = detected_intensity;
            debounce_counter = 0; // Reseta para a próxima vez
        }
    } else {
        debounce_counter = 0;
    }
}

// -------------------------------------
void de_model_handle_phase(dem_t *model)
{
	model->phase = model->next_hr >= model->hr ? RISE : DECAY;
}

// --------------------------------------------
void update_tau(dem_t *model, int deriv_signal)
{
	// Select specific tau for the phase
	float *current_tau = NULL;
	if (model->phase == RISE)
	{
		current_tau = &model->tau_rise;
	}
	else
	{
		current_tau = &model->tau_decay;
	}

	float dot_prod = 0;
	ESP_ERROR_CHECK(dsps_dotprod_f32_aes3(model->ds, model->weights, &dot_prod, WEIGHTS_LEN));
	float error_term = dot_prod - model->next_hr;
	float derivative_term = -expf(-1.0f / *current_tau) / powf(*current_tau, 2);
	float tau_update_term = (model->alpha * derivative_term * error_term);

	if (deriv_signal < 0)
	{
		tau_update_term *= -1;
	}

	model->tau = *current_tau;
	*current_tau = *current_tau - tau_update_term;
}

//------------------------------------------------
void update_weights(dem_t *model, int deriv_signal)
{
	float mul_factor = expf(-1.0f / model->tau);
	float res_op1[WEIGHTS_LEN], res_op2[WEIGHTS_LEN], res_op3[WEIGHTS_LEN], res_final[WEIGHTS_LEN];

	ESP_ERROR_CHECK(dsps_mulc_f32_ae32(model->ds, res_op1, WEIGHTS_LEN, mul_factor, 1, 1));
	ESP_ERROR_CHECK(dsps_sub_f32_ae32(res_op1, model->ds, res_op2, WEIGHTS_LEN, 1, 1, 1));
	ESP_ERROR_CHECK(dsps_mulc_f32_ae32(res_op2, res_op3, WEIGHTS_LEN, model->alpha, 1, 1));

	if (deriv_signal >= 0)
	{
		ESP_ERROR_CHECK(dsps_sub_f32_ae32(model->weights, res_op3, res_final, WEIGHTS_LEN, 1, 1, 1));
	}
	else
	{
		ESP_ERROR_CHECK(dsps_add_f32_ae32(model->weights, res_op3, res_final, WEIGHTS_LEN, 1, 1, 1));
	}

	memcpy(model->weights, res_final, WEIGHTS_LEN * sizeof(float));
}

// ---------------------------------------------
void update_bias(dem_t *model, int deriv_signal)
{
	float update_bias_term = (model->alpha * (expf(-1 / model->tau) - 1));
	if (deriv_signal < 0)
	{
		update_bias_term *= -1;
	}

	if (model->intensity == HIGH)
	{
		model->b_high = model->b_high - update_bias_term;
		return;
	}
	model->b_low = model->b_low - update_bias_term;
}

// -------------------------------------------------
void de_model_partial_fit(dem_t *model, float hr_gt)
{
	int derivative_signal = model->next_hr > hr_gt ? -1 : 1;

	// Do not change order
	update_tau(model, derivative_signal);
	update_weights(model, derivative_signal);
	update_bias(model, derivative_signal);
}

// --------------------------------
void de_model_predict(dem_t *model)
{
	float dot_prod = 0;
	ESP_ERROR_CHECK(dsps_dotprod_f32_aes3(model->ds, model->weights, &dot_prod, WEIGHTS_LEN));

	float bias = model->intensity == HIGH ? model->b_high : model->b_low;
	model->hr_reg = dot_prod + bias;

	float current_tau = model->phase == RISE ? model->tau_rise : model->tau_decay;
	model->hr = model->next_hr; // Update last state
	model->next_hr = (model->hr_reg - ((model->hr_reg - model->next_hr) * expf(-1.0f / current_tau)));
}

// -------------------------------------------
void de_model_debug(dem_t *model, float hr_gt)
{
	printf("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
		   model->weights[0],
		   model->weights[1],
		   model->weights[2],
		   model->weights[3],
		   model->weights[4],
		   model->b_high,
		   model->b_low,
		   model->ds[0], // AL
		   model->tau_rise,
		   model->hr_reg,
		   model->next_hr,
		   hr_gt);
}

// ---------------------------------------------
void de_model_set_als(dem_t *model, float als[3])
{
	model->ds[0] = als[0];
	model->ds[1] = als[1];
	model->ds[2] = als[2];
}
