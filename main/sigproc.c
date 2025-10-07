#include <math.h>
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


void generate_bandpass_FIR_coefficients(float *fir_coeffs, const unsigned int fir_len, const float ft1, const float ft2)
{
    // A ordem do filtro é FIR_COEFFS_LEN - 1
    const float fir_order = (float)(fir_len - 1);
    const bool is_odd = (fir_len % 2) ? (true) : (false);

    // Window coefficients
    float *fir_window = (float *)malloc(fir_len * sizeof(float));
    dsps_wind_blackman_f32(fir_window, fir_len); // Usando Blackman para os coeficientes

    for (int i = 0; i < fir_len; i++) {
        float n_minus_center = i - fir_order / 2.0f;
        
        if (fabsf(n_minus_center) < 1e-6) { // Para n = N/2 (centro)
            // A amplitude no centro é 2*ft2 - 2*ft1 para passa-banda
            fir_coeffs[i] = 2 * (ft2 - ft1);
        } else {
            // Formula passa-banda (bandpass) normalizada:
            // h[n] = 2*ft2 * sinc(2*ft2*n) - 2*ft1 * sinc(2*ft1*n)
            float term2 = sinf(2 * M_PI * ft2 * n_minus_center) / (M_PI * n_minus_center);
            float term1 = sinf(2 * M_PI * ft1 * n_minus_center) / (M_PI * n_minus_center);
            fir_coeffs[i] = term2 - term1;
        }

        fir_coeffs[i] *= fir_window[i];
    }

    free(fir_window);
}
