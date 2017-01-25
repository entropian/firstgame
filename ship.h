#pragma once
#include <GL/glew.h>
#include "mat.h"


class Ship
{
public:
    Ship();
    ~Ship();
    void setUniforms(const Mat4& transform, const Mat4& normal_transform,
                     const Mat4 proj_transform, const Vec3& dir_light_1, const Vec3& dir_light_2);

    void draw();
private:
    GLuint vao, vbo, ibo, shader_program;
    GLuint diffuse_handle, normal_handle;
    unsigned int num_indices;
};
