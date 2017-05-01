#pragma once
#include "vec.h"
#include <float.h>

const Vec3 LEFT(-1.0f, 0.0f, 0.0f);
const Vec3 RIGHT(1.0f, 0.0f, 0.0f);
const Vec3 UP(0.0f, 1.0f, 0.0f);
const Vec3 DOWN(0.0f, -1.0f, 0.0f);
const Vec3 PLUS_Z(0.0f, 0.0f, 1.0f);
const Vec3 MINUS_Z(0.0f, 0.0f, -1.0f);
const Vec3 ORIGIN(0.0f, 0.0f, 0.0f);

const unsigned int FPS = 60;
const float SECONDS_PER_FRAME = 1.0f / (float)FPS;

const float K_EPSILON = 0.0001f;
const float TMAX = FLT_MAX;
const float HUGEVALUE = 1.0E10;

const int NANO_SECONDS_IN_SEC = 1000000000;

// Ship velocity constants
const float MAX_Z_VELOCITY = 20.0f;
const float MAX_X_VELOCITY = 5.0f;
const float MAX_Y_DOWNWARD_VELOCITY = -20.0f;
const float Y_JUMP_VELOCITY = 25.0f;
