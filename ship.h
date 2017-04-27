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
    void updatePosAndVelocity(const float dt, Track& track);
    void calcVelocity(int accel_states[3]);
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
