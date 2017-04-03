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
    void calcGreatestMinPenetration(Vec3& greatest_min_penetration, BBox& new_bbox, const int collided,
        bool opposing_axis[3])
    {
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
                if(fabs(min_penetration) > fabs(greatest_min_penetration[0]))
                {
                    greatest_min_penetration[0] = min_penetration;
                }
                break;
            case 1:
                if(fabs(min_penetration) > fabs(greatest_min_penetration[1]))
                {
                    greatest_min_penetration[1] = min_penetration;
                }
                break;
            case 2:
                if(fabs(min_penetration) > fabs(greatest_min_penetration[2]))
                {
                    greatest_min_penetration[2] = min_penetration;
                }
                break;
            }
        }        
    }
    
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

        int hit_dir = -1;
        float overlap_time = 0.0f;
        for(int i = 0; i < collided; i++)
        {
            Box* track_box = colliding_boxes[i];
            int tmp_hit_dir;
            float tmp_overlap_time = new_bbox.calcOverlapTime(tmp_hit_dir, track_box, velocity);
            if(tmp_overlap_time > overlap_time)
            {
                overlap_time = tmp_overlap_time;
                hit_dir = tmp_hit_dir;
            }
        }

        if(collided > 0)
        {
            dp = velocity * (dt - overlap_time);
            bbox.min += dp;
            bbox.max += dp;
            pos += dp;
            velocity[hit_dir] = 0.0f;
        }
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
