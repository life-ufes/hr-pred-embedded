#include <math.h>
#include <stdio.h>
#include "exp_approx_model.h"
#include "esp_dsp.h"

#define INITIAL_WEIGHT_1 77.19f
#define INITIAL_WEIGHT_2 0.82f
#define INITIAL_WEIGHT_3 -0.9f
#define INITIAL_WEIGHT_4 -6.44f 
#define INITIAL_WEIGHT_5 -0.09f

#define INTENSITY_TRESHOLD 1.25


void ea_model_init(eam_t *model)
{
    float weights[WEIGHTS_LEN] = {
        INITIAL_WEIGHT_1, 
        INITIAL_WEIGHT_2 
        INITIAL_WEIGHT_3
        INITIAL_WEIGHT_4,
        INITIAL_WEIGHT_5
    };

    memcpy(model->weights, weights, WEIGHTS_LEN * sizeof(float));

    model->b_low = 80.0f;
    model->b_high = 120.0f;
    model->tau = 75.0f;
    model->next_tau = 75.0f;
    model->alpha = 5.0f;

    model->hr_reg = 100.0f;
    model->next_hr = 100.0f;

    float ds[WEIGHTS_LEN] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    memcpy(model->ds, ds, WEIGHTS_LEN * sizeof(float));
}


void ea_model_set_user_data(eam_t *model, float bmi, Genre g, int age)
{
    model->ds[1] = bmi;
    model->ds[2] = (float)age;
    model->ds[3] = g == MALE ? 1.0f : 0.0f;
    model->ds[4] = g == FEMALE ? 1.0 : 0.0f; 
}


void ea_model_handle_intensity(eam_t *model)
{
    model->intensity = model->ds[0] >= INTENSITY_TRESHOLD ? HIGH : LOW;
}


// TODO: Review (low variability)
void update_weights(eam_t *model)
{
    float mul_factor = expf(-1.0f/model->tau);
    float res_op1[WEIGHTS_LEN], res_op2[WEIGHTS_LEN], res_op3[WEIGHTS_LEN], res_final[WEIGHTS_LEN];

    ESP_ERROR_CHECK(dsps_mulc_f32_ae32(model->ds, res_op1, WEIGHTS_LEN, mul_factor, 1, 1));
    ESP_ERROR_CHECK(dsps_sub_f32_ae32(res_op1, model->ds, res_op2, WEIGHTS_LEN, 1, 1, 1));
    ESP_ERROR_CHECK(dsps_mulc_f32_ae32(res_op2, res_op3, WEIGHTS_LEN, model->alpha, 1, 1));
    ESP_ERROR_CHECK(dsps_sub_f32_ae32(model->weights, res_op3, res_final, WEIGHTS_LEN, 1, 1, 1));

    memcpy(model->weights, res_final, WEIGHTS_LEN * sizeof(float));
}

// TODO: refactor (values coming to infinity)
void update_tau(eam_t *model)
{
    float factor1 = model->tau - (model->alpha *(-expf(-1/model->tau)/powf(model->tau, 2)));
    
    float dot_prod = 0; 
    dsps_dotprod_f32_aes3(model->ds, model->weights, &dot_prod, WEIGHTS_LEN);
    
    float factor2 = dot_prod - model->next_hr;

    model->tau = model->next_tau;
    model->next_tau = factor1 * factor2;
}


void uptadte_bias(eam_t *model)
{
    if(model->intensity == HIGH){
        model->b_high = model->b_high - (model->alpha * (expf(-1/model->tau) - 1));
        return;
    }
    model->b_low = model->b_low - (model->alpha * (expf(-1/model->tau) - 1));
}


void ea_model_partial_fit(eam_t *model, float al)
{
    // update AL
    model->ds[0] = al;
    
    // Do not change order
    update_tau(model);
    update_weights(model);
    uptadte_bias(model);

    float dot_prod = 0;
    dsps_dotprod_f32_aes3(model->ds, model->weights, &dot_prod, 5);

    float bias = model->intensity == HIGH ? model->b_high : model->b_low;
    model->hr_reg = dot_prod + bias;
}


void ea_model_predict(eam_t *model)
{
    model->next_hr = roundf(model->hr_reg - ((model->hr_reg - model->next_hr) * expf(-1.0f/model->tau)));
}


void ea_model_debug(eam_t *model)
{
    printf("\n\n--- EA MODEL DEBUG ---\n");
    printf("AL: %.2f\n", model->ds[0]);
    printf("Weights: %.2f, %.2f, %.2f, %.2f, %.2f\n", 
        model->weights[0], 
        model->weights[1],
        model->weights[2],
        model->weights[3],
        model->weights[4]
    );
    printf("HR reg.: %.2f\n", model->hr_reg);
    printf("Next HR: %.2f\n", model->next_hr);
    printf("Tau: %.2f\n", model->tau);
    printf("Next Tau: %.2f\n", model->next_tau);
    printf("b_high: %.2f\n", model->b_high);
    printf("b_low: %.2f\n", model->b_low);
    printf("\n\n");
}
