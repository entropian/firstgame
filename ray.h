#pragma once
#include "vec.h"

struct Ray
{
    Vec3 origin;
    Vec3 dir;
    Vec3 calcPoint(const float t)
    {
        return origin + dir * t;
    }            
};
