#pragma once
#include <GL/glew.h>
#include "mat.h"
#include "bbox.h"
#include "track.h"

class Box;

class Ship
{
public:
    Ship();
    ~Ship();
    void setUniforms(const Mat4& model_transform, const Mat4& view_transform, const Mat4& normal_transform,
                     const Mat4 proj_transform, const Vec3& dir_light_1, const Vec3& dir_light_2);
    void setViewTransform(const Mat4& view_transform);
    bool bboxCollide(const BBox& bbox) const;
    BBox getBBox();
    void updatePos(const float dt, Track& track)
    {
        Vec3 dp;
        BBox new_bbox(bbox);
        if(velocity[0] != 0.0f && velocity[1] != 0.0f && velocity[2] != 0.0f)
        {
            dp = velocity * dt;
            new_bbox.min += dp;
            new_bbox.max + dp;
        }
        bool opposing_axis[3];
        int collided = track.bboxCollideWithTrack(colliding_boxes, opposing_axis, new_bbox);
        bool on_surface = false;
        bool stopped = false;

        float greatest_min_x_penetration = 0.0f;
        float greatest_min_y_penetration = 0.0f;
        float greatest_min_z_penetration = 0.0f;

        // If new_bbox collides with track, calculate the distance along the three axes
        // that new_bbox should be moved so that it doesn't collide with the track anymore
        for(int i = 0; i < collided; i++)
        {
            // If ship bbox collids with two boxes on the same axis in different directions,
            // then the ship bbox cannot be moved along that axis to a uncollided space
            // Therefore we skip that axis
            if(opposing_axis[i])
            {
                continue;
            }
            Box* track_box = colliding_boxes[i];

            float min_penetration = 0.0f;
            int min_index;
            min_penetration = new_bbox.calcMinPenetration(min_index, track_box);
            
            switch(min_index)
            {
            case 0:
                if(fabs(min_penetration) > fabs(greatest_min_x_penetration))
                {
                    greatest_min_x_penetration = min_penetration;
                }
                break;
            case 1:
                if(fabs(min_penetration) > fabs(greatest_min_y_penetration))
                {
                    greatest_min_y_penetration = min_penetration;
                }
                break;
            case 2:
                if(fabs(min_penetration) > fabs(greatest_min_x_penetration))
                {
                    greatest_min_x_penetration = min_penetration;
                }
                break;
            }
        }

        // NOTE: margin for y axis?

        // Calculate distance per unit axis
        float x_penetration_time = greatest_min_x_penetration / velocity[0];
        float y_penetration_time = greatest_min_y_penetration / velocity[1];
        float z_penetration_time = greatest_min_z_penetration / velocity[2];        
        float greatest_penetration_time;
        int axis;
        if(x_penetration_time > y_penetration_time)
        {
            greatest_penetration_time = x_penetration_time;
            axis = 0;
        }else
        {
            greatest_penetration_time = y_penetration_time;
            axis = 1;
        }
        if(z_penetration_time > greatest_penetration_time)
        {
            greatest_penetration_time = z_penetration_time;
            axis = 2;
        }
                
        // Move ship back by corresponding distance, then move ship along new velocity
        // in the remaining time after collision
        if(greatest_penetration_time > 0.0f)
        {
            dp += -velocity * greatest_penetration_time;
            // Assume greatest_penetration_time < dt
            float remaining_time = dt - greatest_penetration_time;
            velocity[axis] = 0.0f;
            dp += velocity * remaining_time;
        }
        pos += dp;
    }
    void draw();
private:
    GLuint vao, vbo, ibo, shader_program;
    GLuint diffuse_handle, normal_handle;
    unsigned int num_indices;
    BBox bbox;
    Box* colliding_boxes[24]; // Assuming that each side of the space can collide with up to 4 boxes
    Vec3 velocity;
    Vec3 pos;
};
