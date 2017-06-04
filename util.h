#pragma once
#include <math.h>

float degToRad(const float degree)
{
    return degree / 180.0f * M_PI;
}

float radToDeg(const float radian)
{
    return radian / M_PI * 180.0f;
}
