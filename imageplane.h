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

    void makeOrthographic(const float width, const float height)
    {
        m_aspect_ratio = width / height;
        m_half_frame_width = width * 0.5f;
        m_half_frame_height = height * 0.5f;
    }

    void makePerspective(const float fov, const float aspect_ratio)
    {
        m_fov = fov;
        m_aspect_ratio = aspect_ratio;
        float boost_y = 1.0f / cosf(m_fov * 0.5f * (float)PI / 180.0f);
        m_half_frame_height = sinf(fov * 0.5f * (float)PI / 180.0f) * boost_y;
        m_half_frame_width = m_half_frame_height * m_aspect_ratio;                
    }

    Ray calcOrthographicRay(const float x, const float y)
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
        ret.origin = Vec3(x * m_half_frame_width, y * m_half_frame_height, 0.0f);
        ret.dir = Vec3(0.0f, 0.0f, -1.0f);
        return ret;
    }

    Ray calcPerspectiveRay(const float x, const float y)
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
