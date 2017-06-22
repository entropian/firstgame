#pragma once
#include <cassert>
#include "constants.h"
#include "ray.h"
#include "mat.h"

struct BBox
{
    BBox()
        :min(Vec3(0.0f, 0.0f, 0.0f)), max(Vec3(0.0f, 0.0f, 0.0f))
    {}

    BBox(const Vec3& a, const Vec3& b)
        :min(a), max(b)
    {
        //assert(a[0] <= b[0] && a[1] <= b[1] && a[2] <= b[2]);
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

    virtual void move(const Vec3& v) 
    {
        min += v;
        max += v;
    }

    bool bboxIntersect(const BBox& bbox) const
    {
        if(max[0] < bbox.min[0] || min[0] > bbox.max[0]) return 0;
        if(max[1] < bbox.min[1] || min[1] > bbox.max[1]) return 0;
        if(max[2] < bbox.min[2] || min[2] > bbox.max[2]) return 0;
        return true;
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

    float calcOverlapTime(int& hit_direction, BBox* b, const Vec3 velocity)
    {
        float overlap_time_x = FLT_MAX;
        if(velocity[0] > 0.0f)
        {
            overlap_time_x = fabs((max[0] - b->min[0]) / velocity[0]);
        }else if(velocity[0] < 0.0f)
        {
            overlap_time_x = fabs((min[0] - b->max[0]) / velocity[0]);
        }

        float overlap_time_y = FLT_MAX;
        if(velocity[1] > 0.0f)
        {
            overlap_time_y = fabs((max[1] - b->min[1]) / velocity[1]);
        }else if(velocity[1] < 0.0f)
        {
            overlap_time_y = fabs((min[1] - b->max[1]) / velocity[1]);
        }

        float overlap_time_z = FLT_MAX;
        if(velocity[2] > 0.0f)
        {
            overlap_time_z = fabs((max[2] - b->min[2]) / velocity[2]);
        }else if(velocity[2] < 0.0f)
        {
            overlap_time_z = fabs((min[2] - b->max[2]) / velocity[2]);
        }
        
        float least_overlap_time = 0.0f;
        if(overlap_time_x < overlap_time_y)
        {
            least_overlap_time = overlap_time_x;
            if(velocity[0] < 0)
            {
                hit_direction = 0;
            }else
            {
                hit_direction = 1;
            }
        }else
        {
            least_overlap_time = overlap_time_y;
            if(velocity[1] < 0)
            {
                hit_direction = 2;
            }else
            {
                hit_direction = 3;
            }
        }

        if(overlap_time_z < least_overlap_time)
        {
            least_overlap_time = overlap_time_z;
            if(velocity[2] < 0)
            {
                hit_direction = 4;
            }else
            {
                hit_direction = 5;
            }
        }
        return least_overlap_time;
    }

    // TODO: delete this?
    float calcMinPenetration(int& axis_index, const BBox* b)
    {
        // Determine the absolute value of penetration in each axis
        float min_penetration = 0.0f;
        int min_index = -1;
        for(int j = 0; j < 3; j++)
        {
            float penetration = 0.0f;
            if(min[j] < b->min[j] && max[j] < b->max[j])
            {
                // From left side
                penetration = max[j] - b->min[j];
            }else if(max[j] > b->max[j] && min[j] > b->min[j])
            {
                // From right side
                penetration = min[j] - b->max[j];
            }else if(min[j] > b->min[j] && max[j] < b->max[j])
            {
                // Inside
                float min_diff = min[j] - b->min[j];
                float max_diff = max[j] - b->max[j];
                if(fabs(min_diff) < fabs(max_diff))
                {
                    penetration = min_diff;
                }else
                {
                    penetration = max_diff;
                }
            }else if(min[j] < b->min[j] && max[j] > b->max[j])
            {
                // Outside
                float diff1 = min[j] - b->max[j];
                float diff2 = max[j] - b->min[j];
                if(fabs(diff1) < fabs(diff2))
                {
                    penetration = diff1;
                }else
                {
                    penetration = diff2;
                }
            }
            if(fabs(penetration) < fabs(min_penetration))
            {
                min_penetration = penetration;
                min_index = j;
            };
        }
        axis_index = min_index;
        return min_penetration;
    }

    Vec3 min, max;
};
