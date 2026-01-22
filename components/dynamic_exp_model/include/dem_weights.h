#pragma once

#include "sdkconfig.h"

// ==========================================
// ACC
// ==========================================
#if defined(CONFIG_SENSOR_ACC)
#define INITIAL_WEIGHT_1 90.0f
#define B_HIGH 70.0f
#define B_LOW 80.0f

// ==========================================
// GYRO
// ==========================================
#elif defined(CONFIG_SENSOR_GYRO)
#define INITIAL_WEIGHT_1 56.25f
#define B_HIGH 80.0f
#define B_LOW 60.0f

#endif

// ==========================================
// SHARED
// ==========================================
#define INITIAL_WEIGHT_2 -10.0f
#define INITIAL_WEIGHT_3  35.0f
#define INITIAL_WEIGHT_4  0.80f
#define INITIAL_WEIGHT_5 -0.70f
#define INITIAL_WEIGHT_6 -0.60f

#define TAU_RISE 20.0f      // The rise is slower than the decay
#define TAU_DECAY 40.0f

// #define L_RATE 0.005f
#define L_RATE 0.02f
#define INTENSITY_THRESHOLD 0.2f
#define INTENSITY_DEBOUNCE_LIMIT 5
