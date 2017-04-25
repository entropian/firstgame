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
    void setStaticUniforms(const Mat4& proj_transform, const Vec3& dir_light_1, const Vec3& dir_light_2);
    void setViewTransform(const Mat4& view_transform);
    void updateDynamicUniforms(const Mat4& view_transform);
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

    void updateVelocity(const int velocity_change[3])
    {
        const float x_speed = 2.0f;
        const float y_speed = 2.0f;
        const float z_speed = -5.0f;
        if(velocity_change[0] == 1)
        {
            velocity[0] = x_speed;
        }else if(velocity_change[0] == -1)
        {
            velocity[0] = -x_speed;
        }else if(velocity_change[0] == 0)
        {
            velocity[0] = 0.0f;
        }


        if(velocity_change[2] == 1)
        {
            velocity[2] = z_speed;
        }else if(velocity_change[2] == -1)
        {
            velocity[2] = -z_speed;
        }else if(velocity_change[2] == 0)
        {
            velocity[2] = 0.0f;
        }        

    }
    
    void updatePosAndVelocity(const float dt, Track& track)
    {
        Vec3 dp;
        BBox new_bbox(bbox);
        if(velocity[0] != 0.0f || velocity[1] != 0.0f || velocity[2] != 0.0f)
        {
            dp = velocity * dt;
            new_bbox.min += dp;
            new_bbox.max += dp;
        }

        int collided = track.bboxCollideWithTrack(colliding_boxes, new_bbox);
        //printf("collided %d\n", collided);

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

        //if(collided > 0)
        {
            dp = velocity * (dt - overlap_time);
            bbox.min += dp;
            bbox.max += dp;
            //pos += dp;
            if(overlap_time > 0.0f)
            {
                velocity[hit_dir] = 0.0f;
            }
        }

        Mat4 displacement = Mat4::makeTranslation(dp);
        //printf("dp %f, %f, %f\n", dp[0], dp[1], dp[2]);
        //displacement.print();
        transform = displacement * transform;
    }
    void draw();
//private:
    GLuint vao, vbo, ibo, shader_program;
    // Texture handles
    GLuint diffuse_map, normal_map;
    // Shader dynamic uniforms
    GLuint u_model_mat, u_normal_mat, u_view_mat;
    
    unsigned int num_indices;
    BBox bbox;
    Box* colliding_boxes[24]; // Assuming that each side of the space can collide with up to 4 boxes
    Vec3 velocity;
    Mat4 transform;    
};
