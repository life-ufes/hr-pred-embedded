#pragma once

#define WEIGHTS_LEN 5

/// @brief Enum for exercise intensity levels
typedef enum
{
    HIGH,
    LOW
} intensity_eam_t;

/// @brief Structure for the Exponential Approximation Model
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
    intensity_eam_t intensity;

} __attribute__((aligned(16))) eam_t;

/// @brief Initializes the Exponential Approximation Model
/// @param model: Pointer to the model to initialize
void ea_model_init(eam_t *model);

/// @brief Sets user data for the model
/// @param model: Pointer to the model
/// @param bmi: Body Mass Index
/// @param age: Age in years
/// @param male: Male
/// @param female: Female
void ea_model_set_user_data(eam_t *model, float bmi, int age, float male, float female);

/// @brief Handles intensity detection with debounce logic
/// @param model: Pointer to the model
/// @param al_raw: Raw activity level
void ea_model_handle_intensity(eam_t *model, float al_raw);

/// @brief Performs a partial fit of the model based on ground truth heart rate
/// @param model: Pointer to the model
/// @param hr_gt: Ground truth heart rate
void ea_model_partial_fit(eam_t *model, float hr_gt);

/// @brief Predicts the next heart rate using the model
/// @param model: Pointer to the model
void ea_model_predict(eam_t *model);

/// @brief Debug function to print model parameters
/// @param model: Pointer to the model
/// @param hr_gt: Ground truth heart rate
void ea_model_debug(eam_t *model, float hr_gt);

/// @brief Sets the activity level in the model
/// @param model: Pointer to the model
/// @param al: Activity level normalized value
void ea_model_set_al(eam_t *model, float al);
