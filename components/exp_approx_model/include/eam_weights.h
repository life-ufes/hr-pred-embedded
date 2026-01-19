#pragma once

#include "sdkconfig.h"

// ==========================================
// ACC
// ==========================================
#if defined(CONFIG_SENSOR_ACC)
#define INITIAL_WEIGHT_1 93.75f
#define B_HIGH 60.0f
#define B_LOW 40.0f

// #define INITIAL_WEIGHT_1 77.19f
// #define B_HIGH 120.0f
// #define B_LOW 80.0f


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
#define INITIAL_WEIGHT_2 0.82f
#define INITIAL_WEIGHT_3 -0.90f
#define INITIAL_WEIGHT_4 -6.44f
#define INITIAL_WEIGHT_5 -0.09f

#define TAU 56.25f
// #define TAU 75.0f


// #define L_RATE 0.01f
#define L_RATE 0.0092f
#define INTENSITY_THRESHOLD 1.0f
#define INTENSITY_DEBOUNCE_LIMIT 5
