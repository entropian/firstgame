#pragma once

#include "mat.h"
#include "imageplane.h"
#include "util.h"

enum Direction
{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera
{
public:
    Camera()
    {
    }

    Camera(const float fov, const float aspect_ratio)
        : m_image_plane(fov, aspect_ratio), pos(Vec3(0.0f, 0.0f, 0.0f)), euler_angle(0.0f, 0.0f, 0.0f) {}

    Camera(const Vec3& dir, const Vec3& up_vec, const Vec3& p, const float fov, const float aspect_ratio)
        :m_image_plane(fov, aspect_ratio), pos(p)
    {
        camera_transform = lookAt(dir, up_vec, p);
        view_transform = Mat4::makeTranslation(Vec3(0.0f, 0.0f, 1.0f)) * camera_transform.inverse();
        float phi = asin(dir[1]);
        euler_angle[1] = radToDeg(phi);
        float cos_phi = cosf(phi);
        float x_over_cos_phi = dir[0] / cos_phi;
        float z_over_cos_phi = dir[2] / cos_phi;
        float theta = asin(x_over_cos_phi);
        euler_angle[0] = radToDeg(theta);
    }

    Mat4 lookAt(const Vec3& dir, const Vec3& up_vec, const Vec3& p)
    {
        Mat4 translation = Mat4::makeTranslation(p);
        Vec3 z = normalize(-dir);
        Vec3 x = normalize(cross(up_vec, z));
        Vec3 y = cross(z, x);
        Mat4 orientation;
        for(int i = 0; i < 3; i++)
        {
            orientation(i, 0) = x[i];
            orientation(i, 1) = y[i];
            orientation(i, 2) = z[i];
        }
        return translation * orientation;
    }    

    void updateTransforms()
    {
        // Default euler angle (0, 0, 0) points down negative z axis
        // Roll is not used
        // Calculate camera direction vector
        Vec3 dir;
        dir[0] = sinf(degToRad(euler_angle[0]));
        dir[2] = -cosf(degToRad(euler_angle[0]));

        dir[1] = sinf(degToRad(euler_angle[1]));
        dir[0] *= cosf(degToRad(euler_angle[1]));
        dir[2] *= cosf(degToRad(euler_angle[1]));
        dir.normalize();

        camera_transform = lookAt(dir, Vec3(0.0f, 1.0f, 0.0f), pos);
        view_transform = Mat4::makeTranslation(Vec3(0.0f, 0.0f, 1.0f)) * camera_transform.inverse();
    }

    void move(const Direction direction, const float distance)
    {
        Vec3 move_dir;
        switch(direction)
        {
        case LEFT:
        {
            move_dir = -getXAxis();
        } break;
        case RIGHT:
        {
            move_dir = getXAxis();
        } break;
        case UP: 
        {
            move_dir = Vec3(0.0f, 1.0f, 0.0f);
        } break;
        case DOWN:
        {
            move_dir = Vec3(0.0f, -1.0f, 0.0f);
        } break;
        case FORWARD:
        {
            move_dir = -getZAxis();
        } break;
        case BACKWARD: 
        {
            move_dir = getZAxis();
        } break;
        }
        
        Vec3 displacement = move_dir * distance;
        for(int i = 0; i < 3; i++)
        {
            pos[i] += displacement[i];
            camera_transform(i, 3) += displacement[i];
        }
        view_transform = Mat4::makeTranslation(Vec3(0.0f, 0.0f, 1.0f)) * camera_transform.inverse();
    }

    void turn(const float heading, const float pitch)
    {
        euler_angle[0] += heading;
        while(euler_angle[0] > 180.0f)
        {
            euler_angle[0] -= 360.0f;
        }
        while(euler_angle[0] < -180.0f)
        {
            euler_angle[0] += 360.0f;
        }
        euler_angle[1] += pitch;
        if(euler_angle[1] > 89.0f)
        {
            euler_angle[1] = 89.0f;
        }
        if(euler_angle[1] < -89.0f)
        {
            euler_angle[1] = -89.0f;
        }
        updateTransforms();
    }

    Ray transformRay(const Ray& ray)
    {
        Ray ret;
        ret.origin = camera_transform * Vec4(ray.origin, 1.0f);
        ret.dir = camera_transform * Vec4(ray.dir, 0.0f);
        return ret;
    }

    Ray calcRayFromScreenCoord(const float x, const float y)
    {
        Ray ray = m_image_plane.calcRay(x, y);
        ray = this->transformRay(ray);
        return ray;
    }

    void setPosRelativeToShip(const Ship& ship)
    {
        Vec3 ship_pos = ship.getPos();
        Vec3 displacement = ship_pos - pos;
        displacement[0] = 0.0f;
        displacement[1] = 0.0f;
        const float cam_to_ship_z_dist = 5.0f;
        displacement[2] += cam_to_ship_z_dist;

        pos[0] = 0.0f;
        // TODO: put game mode camera height in a variable somewheren
        pos[1] = 1.0f;
        pos[2] += displacement[2];
        camera_transform(0, 3) = pos[0];
        camera_transform(1, 3) = pos[1];
        camera_transform(2, 3) = pos[2];
        view_transform = Mat4::makeTranslation(Vec3(0.0f, 0.0f, 1.0f)) * camera_transform.inverse();        
    }

    Mat4 getViewTransform() const
    {
        return view_transform;
    }
    
    Mat4 getCameraTransform() const
    {
        return camera_transform;
    }

    Vec3 getPosition() const
    {
        return pos;
    }

    Vec3 getXAxis() const
    {
        return Vec3(camera_transform(0, 0), camera_transform(1, 0), camera_transform(2, 0));
    }

    Vec3 getYAxis() const
    {
        return Vec3(camera_transform(0, 1), camera_transform(1, 1), camera_transform(2, 1));
    }

    Vec3 getZAxis() const
    {
        return Vec3(camera_transform(0, 2), camera_transform(1, 2), camera_transform(2, 2));
    }

    void setPosAndOrientation(const Vec3& cam_pos, const Vec3& cam_euler_ang)
    {
        euler_angle = cam_euler_ang;
        pos = cam_pos;
        updateTransforms();
    }

    Vec3 getEulerAng()
    {
        return euler_angle;
    }
private:
    Mat4 view_transform;
    Mat4 camera_transform;
    Vec3 scale;
    ImagePlane m_image_plane;
    Vec3 euler_angle;
    Vec3 pos;
};
