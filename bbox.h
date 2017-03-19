#pragma once
#include <cassert>
#include "constants.h"
#include "ray.h"
#include "mat.h"

class BBox
{
public:
    BBox()
        :min(Vec3(0.0f, 0.0f, 0.0f)), max(Vec3(0.0f, 0.0f, 0.0f))
    {}

    BBox(const Vec3& a, const Vec3& b)
        :min(a), max(b)
    {
        assert(a[0] <= b[0] && a[1] <= b[1] && a[2] <= b[2]);
        min = a;
        max = b;
    }

    void enlargeTo(const Vec3& point)
    {
        for(int i = 0; i < 3; i++)
        {
            if(min[i] > point[i])
            {
                min[i] = point[i];
            }
            if(max[i] < point[i])
            {
                max[i] = point[i];
            }
        }
    }

    void transform(const Mat4& m)
    {
        Vec4 corners[8];
        corners[0] = Vec4(min[0], min[1], min[2], 1.0f); // Back bottom left
        corners[1] = Vec4(max[0], min[1], min[2], 1.0f); // Back bottom right
        corners[2] = Vec4(min[0], max[1], min[2], 1.0f); // Back top left
        corners[3] = Vec4(max[0], max[1], min[2], 1.0f); // Back top right
        corners[0] = Vec4(min[0], min[1], max[2], 1.0f); // Front bottom left
        corners[1] = Vec4(max[0], min[1], max[2], 1.0f); // Front bottom right
        corners[2] = Vec4(min[0], max[1], max[2], 1.0f); // Front top left
        corners[3] = Vec4(max[0], max[1], max[2], 1.0f); // Front top right

        min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
        max = Vec3(-HUGEVALUE, -HUGEVALUE, -HUGEVALUE);
        for(int i = 0; i < 8; i++)
        {
            corners[i] = m * corners[i];
            this->enlargeTo(Vec3(corners[i]));
        }        
    }
    
    float rayIntersect(int& face, const Ray& ray) const
    {
        float tx_min, ty_min, tz_min;
        float tx_max, ty_max, tz_max;

        float a = 1.0f/ray.dir[0];
        if(a >= 0)
        {
            tx_min = (min[0] - ray.origin[0]) * a;
            tx_max = (max[0] - ray.origin[0]) * a;
        }else
        {
            tx_min = (max[0] - ray.origin[0]) * a;
            tx_max = (min[0] - ray.origin[0]) * a;        
        }

        float b = 1.0f/ray.dir[1];
        if(b >= 0)
        {
            ty_min = (min[1] - ray.origin[1]) * b;
            ty_max = (max[1] - ray.origin[1]) * b;        
        }else
        {
            ty_min = (max[1] - ray.origin[1]) * b;
            ty_max = (min[1] - ray.origin[1]) * b;        
        }

        float c = 1.0f/ray.dir[2];
        if(c >= 0)
        {
            tz_min = (min[2] - ray.origin[2]) * c;
            tz_max = (max[2] - ray.origin[2]) * c;
        }else
        {
            tz_min = (max[2] - ray.origin[2]) * c;
            tz_max = (min[2] - ray.origin[2]) * c;
        }
    
        float t0, t1;
        int face_in = -1, face_out = -1;
        if(tx_min > ty_min)
        {
            t0 = tx_min;
            face_in = (a >= 0.0f) ? 1 : 0;
        }else
        {
            t0 = ty_min;
            face_in = (b >= 0.0f) ? 3 : 2;
        }

        if(tz_min > t0)
        {
            t0 = tz_min;
            face_in = (c >= 0.0f) ? 5 : 4;
        }

        if(tx_max < ty_max)
        {
            t1 = tx_max;
            face_out = (a >= 0.0f) ? 0 : 1;
        }else
        {
            t1 = ty_max;
            face_out = (b >= 0.0f) ? 2 : 3;
        }

        if(tz_max < t1)
        {
            t1 = tz_max;
            face_out = (c >= 0.0f ) ? 4 : 5;
        }

        if(t0 < t1 && t1 > K_EPSILON)
        {
            if(t0 > K_EPSILON)
            {
                face = face_in;
                return t0;
            }else
            {
                face = face_out;
                return t1;
            }
        }else
        {
            return TMAX;
        }
    }
protected:
    Vec3 min, max;
};
