#include <math.h>
#include <stdio.h>
#include "exp_approx_model.h"
#include "esp_dsp.h"

void update_weights(eam_t *model)
{
    float mul_factor = expf(-1.0f/model->tau);
    float res_op1[W_LEN], res_op2[W_LEN], res_op3[W_LEN], res_final[W_LEN];

    ESP_ERROR_CHECK(dsps_mulc_f32_ae32(model->ds, res_op1, W_LEN, mul_factor, 1, 1));
    ESP_ERROR_CHECK(dsps_sub_f32_ae32(res_op1, model->ds, res_op2, W_LEN, 1, 1, 1));
    ESP_ERROR_CHECK(dsps_mulc_f32_ae32(res_op2, res_op3, W_LEN, model->alpha, 1, 1));
    ESP_ERROR_CHECK(dsps_sub_f32_ae32(model->weights, res_op3, res_final, W_LEN, 1, 1, 1));

    memcpy(model->weights, res_final, W_LEN);
}

void update_tau(eam_t *model)
{
    return;
}

void uptadte_bias(eam_t *model)
{
    return;
}

void ea_model_init()
{
    return;
}

void ea_model_fit(eam_t *model)
{
    update_weights(model);

    float dot_prod = 0;
    dsps_dotprod_f32_aes3(model->ds, model->weights, &dot_prod, 5);

    // Still using blow
    model->hr_til = dot_prod + model->b_low;
}

int ea_model_predict(eam_t *model)
{
    return (int)round(model->hr_til - ((model->hr_til - model->hr) * expf(-1.0f/model->tau)));
}
