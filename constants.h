#pragma once
#include "vec.h"
#include <float.h>

const Vec3 ORIGIN(0.0f, 0.0f, 0.0f);

const unsigned int FPS = 60;
const float SECONDS_PER_FRAME = 1.0f / (float)FPS;

const float K_EPSILON = 0.0001f;
const float TMAX = FLT_MAX;
const float HUGEVALUE = 1.0E10;

const int NANO_SECONDS_IN_SEC = 1000000000;

