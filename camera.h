#pragma once

#include "mat.h"

class Camera
{
public:
    Camera()
    {
    }

    Camera(const Vec3& dir, const Vec3& up_vec, const Vec3& pos)
    {
        translation = Mat4::makeTranslation(pos);
        // OpenGL flips the z axis around
        Vec3 z = normalize(-dir);
        Vec3 x = normalize(cross(up_vec, z));
        Vec3 y = cross(z, x);
        for(int i = 0; i < 3; i++)
        {
            orientation(i, 0) = x[i];
            orientation(i, 1) = y[i];
            orientation(i, 2) = z[i];
        }
        camera_transform = translation * orientation;
        // NOTE: it looks like that image plane of the camera is at the origin instead of at z = -1
        view_transform = Mat4::makeTranslation(Vec3(0.0f, 0.0f, 1.0f)) * camera_transform.inverse();
    }

    void move(const unsigned int dir, const float distance)
    {
        Vec3 move_dir;
        switch( dir)
        {
        case 0: // Positive x
        {
            move_dir = getXAxis();
        } break;
        case 1: // Negative x
        {
            move_dir = -getXAxis();
        } break;
        case 2: // Positive y
        {
            move_dir = getYAxis();
        } break;
        case 3: // Negative y
        {
            move_dir = -getYAxis();
        } break;
        case 4: // Positive z
        {
            move_dir = -getZAxis();
        } break;
        case 5: // Negative z
        {
            move_dir = getZAxis();
        } break;
        }
        
        Vec3 displacement = move_dir * distance;
        for(int i = 0; i < 3; i++)
        {
            // TODO: make translation a Vec3
            translation(i, 3) += displacement[i];
            camera_transform(i, 3) += displacement[i];
            view_transform = Mat4::makeTranslation(Vec3(0.0f, 0.0f, 1.0f)) * camera_transform.inverse();
        }
    }

    void turnSideways(const float degrees)
    {
        Mat4 rotation = Mat4::makeYRotation(degrees);
        orientation = orientation * rotation;
        camera_transform = translation * orientation;
        Vec3 z(orientation(2, 0), orientation(2, 1), orientation(2, 2));
        view_transform = Mat4::makeTranslation(Vec3(0.0f, 0.0f, 1.0f)) *
            orientation.inverse() * translation.inverse();
    }

    Mat4 view_transform;
    Mat4 camera_transform;
    Mat4 orientation;
    Mat4 translation;
    Vec3 scale;

private:
    Vec3 getXAxis()
    {
        return Vec3(orientation(0, 0), orientation(1, 0), orientation(2, 0));
    }

    Vec3 getYAxis()
    {
        return Vec3(orientation(0, 1), orientation(1, 1), orientation(2, 1));
    }

    Vec3 getZAxis()
    {
        return Vec3(orientation(0, 2), orientation(1, 2), orientation(2, 2));
    }    
};
