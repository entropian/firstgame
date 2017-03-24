#pragma once
#include <GL/glew.h>
#include "mat.h"
#include "bbox.h"

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
    void draw();
private:
    GLuint vao, vbo, ibo, shader_program;
    GLuint diffuse_handle, normal_handle;
    unsigned int num_indices;
    BBox bbox;
    Box* colliding_boxes[24]; // Assuming that each side of the space can collide with up to 4 boxes
    Vec3 velocity;
};
