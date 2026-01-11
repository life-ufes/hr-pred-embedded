#pragma once

#include "sdkconfig.h"

// ==========================================
// ACC
// ==========================================
#if defined(CONFIG_DEM_SENSOR_ACC)
#define INITIAL_WEIGHT_1 110.0f
#define B_HIGH 70.0f
#define B_LOW 50.0f

// ==========================================
// GYRO
// ==========================================
#elif defined(CONFIG_DEM_SENSOR_GYRO)
#define INITIAL_WEIGHT_1 45.0f
#define B_HIGH 90.0f
#define B_LOW 70.0f

#endif

// ==========================================
// SHARED
// ==========================================
#define INITIAL_WEIGHT_2 0.82f
#define INITIAL_WEIGHT_3 -0.90f
#define INITIAL_WEIGHT_4 -6.44f
#define INITIAL_WEIGHT_5 -0.09f
#define INITIAL_WEIGHT_6 -0.09f

#define TAU_RISE 20.0f
#define TAU_DECAY 80.0f

#define L_RATE 0.005f
#define INTENSITY_THRESHOLD 1.0f
