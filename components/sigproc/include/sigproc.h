#pragma once

/// @brief Calculate acceleration magnitude vector from x, y, z components
/// @param x: X component
/// @param y: Y component
/// @param z: Z component
/// @param out: Output magnitude vector
/// @param len: Length of the vectors
void calc_accel_mag_vec(float *x, float *y, float *z, float *out, int len);

/// @brief Exponentially weighted moving average filter context
typedef struct ewma_t
{
    float alpha;
    float last_value;
} ewma_t;

/// @brief Update an exponentially weighted moving average filter
/// @param filter: filter context
/// @param input: new input value
/// @return filtered value
float ewma_update(ewma_t *const filter, const float input);

/// @brief Trapezoidal integral context
typedef struct
{
    float prev;
    float dt;
} trapz_ctx_t;

/// @brief Calculate the trapezoidal integral of a signal
/// @param ctx: integral context
/// @param signal: input signal
/// @param len: length of the signal
/// @return integral value
float trapz_integral(trapz_ctx_t *const ctx, const float *signal, const int len);

/// @brief Clip a signal to a maximum value
/// @param agg_signal: signal to be clipped
/// @param len: length of the signal
/// @param cut: maximum value
void clip(float *agg_signal, int len, float cut);
