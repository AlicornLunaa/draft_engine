#pragma once

#define PI 3.141592654f
#define TO_RAD(x) (x * (PI / 180.f))
#define TO_DEG(x) (x * (180.f / PI))

#define CONSOLE_MAX_LINES 100

#define TIME_STEP 1.f/60.f
#define VELOCITY_ITER 6
#define POSITION_ITER 2
#define PPM 128.f
#define PHYS_TO_PIXELS(x) (x * PPM)
#define PIXELS_TO_PHYS(x) (x / PPM)

// #define DEBUG true
// #define ANSI_SUPPORT