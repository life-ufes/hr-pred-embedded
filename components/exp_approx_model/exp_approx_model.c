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

    if (detected_intensity != model->intensity)
    {
        debounce_counter++;

        if (debounce_counter >= INTENSITY_DEBOUNCE_LIMIT)
        {
            model->intensity = detected_intensity;
            debounce_counter = 0; // Reset for the next step
        }
    }
    else
    {
        debounce_counter = 0;
    }
}

// ------------------------------------------------
void update_weights(eam_t *restrict model, int deriv_signal)
{
    float mul_factor = expf(-1.0f / model->tau);
    float scalar_k = model->alpha * (mul_factor - 1.0f);

    if (deriv_signal >= 0)
    {
        scalar_k = -scalar_k;
    }

    for (int i = 0; i < WEIGHTS_LEN; i++)
    {
        model->weights[i] += model->ds[i] * scalar_k;
    }
}

// --------------------------------------------
void update_tau(eam_t *model, int deriv_signal)
{
    float dot_prod = 0;

    dsps_dotprod_f32_aes3(model->ds, model->weights, &dot_prod, WEIGHTS_LEN);

    float error_term = dot_prod - model->next_hr;
    float tau_sq = model->next_tau * model->next_tau;

    float derivative_term = -expf(-1.0f / model->next_tau) / tau_sq;
    float tau_update_term = (model->alpha * derivative_term * error_term);

    if (deriv_signal < 0)
    {
        tau_update_term *= -1;
    }

    float next_tau_calc = model->next_tau - tau_update_term;

    model->tau = model->next_tau;
    model->next_tau = next_tau_calc;
}

// ---------------------------------------------
void update_bias(eam_t *model, int deriv_signal)
{
    float update_bias_term = (model->alpha * (expf(-1.0f / model->tau) - 1.0f));

    if (deriv_signal < 0)
    {
        update_bias_term *= -1;
    }

    if (model->intensity == HIGH)
    {
        model->b_high -= update_bias_term;
    }
    else
    {
        model->b_low -= update_bias_term;
    }
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
    dsps_dotprod_f32_aes3(model->ds, model->weights, &dot_prod, WEIGHTS_LEN);

    float bias = (model->intensity == HIGH) ? model->b_high : model->b_low;
    model->hr_reg = dot_prod + bias;

    float exp_val = expf(-1.0f / model->next_tau);
    model->next_hr = model->hr_reg - (model->hr_reg - model->next_hr) * exp_val;
}

// -----------------------------------------
void ea_model_set_al(eam_t *model, float al)
{
    model->ds[0] = al;
}
