#include <cstdio>
#include <cmath>
#include "vec.h"
#include "constants.h"
#include "ray.h"

class ImagePlane
{
public:
    ImagePlane()
    {}

    ImagePlane(const float fov, const float aspect_ratio)
        :m_fov(fov), m_aspect_ratio(aspect_ratio)
    {
        float boost_y = 1.0f / cosf(m_fov * 0.5f * (float)PI / 180.0f);
        m_half_frame_height = sinf(fov * 0.5f * (float)PI / 180.0f) * boost_y;
        m_half_frame_width = m_half_frame_height * m_aspect_ratio;        
    }

    Ray calcRay(const float x, const float y)
    {
        if(x > 1.0f || x < -1.0f)
        {
            fprintf(stderr, "x out of bound.\n");
        }
        if(y > 1.0f || y < -1.0f)
        {
            fprintf(stderr, "y out of bound.\n");
        }

        Ray ret;
        ret.dir[0] = x * m_half_frame_width;
        ret.dir[1] = y * m_half_frame_height;
        ret.dir[2] = -1.0f;
        ret.dir = ret.dir.normalize();
        ret.origin = ORIGIN;
        return ret;
    }
    
private:
    float m_fov, m_aspect_ratio;
    float m_half_frame_width, m_half_frame_height;
};
