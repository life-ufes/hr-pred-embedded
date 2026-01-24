#include <math.h>
#include <stdio.h>
#include "esp_dsp.h"
#include "exp_approx_model.h"
#include "eam_weights.h"

// ----------------------------
void ea_model_init(eam_t *model)
{
    float weights[WEIGHTS_LEN] = {
        INITIAL_WEIGHT_1,
        INITIAL_WEIGHT_2,
        INITIAL_WEIGHT_3,
        INITIAL_WEIGHT_4,
        INITIAL_WEIGHT_5};

    memcpy(model->weights, weights, WEIGHTS_LEN * sizeof(float));

    model->b_low = B_LOW;
    model->b_high = B_HIGH;
    model->tau = TAU;
    model->next_tau = TAU;
    model->alpha = L_RATE;

    model->hr_reg = 85.0f;
    model->next_hr = 85.0f;

    float ds[WEIGHTS_LEN] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    memcpy(model->ds, ds, WEIGHTS_LEN * sizeof(float));
}


// -------------------------------------------------------------------
void ea_model_set_user_data(eam_t *model, float bmi, int age, float male, float female)
{
    model->ds[1] = bmi;
    model->ds[2] = (float)age;
    model->ds[3] = male;
    model->ds[4] = female;
}


// ----------------------------------------
void ea_model_handle_intensity(eam_t *model, float al_raw)
{
    static int debounce_counter = 0;
    
    intensity_eam_t detected_intensity = (al_raw >= INTENSITY_THRESHOLD) ? HIGH : LOW;

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


// ------------------------------------------------
void update_weights(eam_t *model, int deriv_signal)
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


// --------------------------------------------
void update_tau(eam_t *model, int deriv_signal)
{
    float dot_prod = 0;
    ESP_ERROR_CHECK(dsps_dotprod_f32_aes3(model->ds, model->weights, &dot_prod, WEIGHTS_LEN));

    float error_term = dot_prod - model->next_hr;
    float derivative_term = -expf(-1.0f / model->next_tau) / powf(model->next_tau, 2);
    float tau_update_term = (model->alpha * derivative_term * error_term);

    if (deriv_signal < 0)
    {
        tau_update_term *= -1;
    }

    float next_tau_calc = model->next_tau - tau_update_term;

    // Update state
    model->tau = model->next_tau;
    model->next_tau = next_tau_calc;
}


// ---------------------------------------------
void update_bias(eam_t *model, int deriv_signal)
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


// -----------------------------------------------------------
void ea_model_partial_fit(eam_t *model, float hr_gt)
{
    int derivative_signal = model->next_hr > hr_gt ? -1 : 1;

    // Do not change order
    update_tau(model, derivative_signal);
    update_weights(model, derivative_signal);
    update_bias(model, derivative_signal);
}


// --------------------------------
void ea_model_predict(eam_t *model)
{
    float dot_prod = 0;
    ESP_ERROR_CHECK(dsps_dotprod_f32_aes3(model->ds, model->weights, &dot_prod, WEIGHTS_LEN));
    
    float bias = model->intensity == HIGH ? model->b_high : model->b_low;
    model->hr_reg = dot_prod + bias;

    model->next_hr = (model->hr_reg - ((model->hr_reg - model->next_hr) * expf(-1.0f / model->next_tau)));
}


// -------------------------------------------
void ea_model_debug(eam_t *model, float hr_gt)
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
           model->next_tau,
           model->hr_reg,
           model->next_hr,
           hr_gt);
}


// -----------------------------------------
void ea_model_set_al(eam_t *model, float al)
{
    model->ds[0] = al;
}
