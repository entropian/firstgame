#pragma once

#include <vector>
#include <cassert>
#include "vec.h"
#include "shaders/shader.h"
#include <float.h>
#include "bbox.h"
#include <fstream>
#include <sstream>

static float snapToUnit(const float f)
{
    float grid_coord = f / GRID_UNIT;
    float grid_floor = floorf(grid_coord);
    float grid_ceil = ceilf(grid_coord); 
    if(f - grid_floor < grid_ceil - f) 
    {
        return grid_floor * GRID_UNIT;
    }else
    {
        return grid_ceil * GRID_UNIT;
    }
}

static Vec3 snapToGrid(const Vec3& v)
{
    return Vec3(snapToUnit(v[0]), snapToUnit(v[1]), snapToUnit(v[2]));
}

/*
  Axially aligned box
  min and max are snapped to multiples of GRID_UNIT
  continuous_min and continuous_max keep track of the "real" min and max
 */
class Box : public BBox
{
public:
    Box()
        :BBox()
    {
    }
    
    Box(const Vec3& a, const Vec3& b)
        :BBox(snapToGrid(a), snapToGrid(b)), color(1.0f, 1.0f, 1.0f)
    {
        continuous_min = min;
        continuous_max = max;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        constructBox();
        glBindVertexArray(0);        
        num_vertices = 36;        
    }
    Box(const Vec3& a, const Vec3& b, const Vec3& c)
        :BBox(snapToGrid(a), snapToGrid(b)), color(c)
    {
        continuous_min = min;
        continuous_max = max;
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        constructBox();
        glBindVertexArray(0);        
        num_vertices = 36;        
    }    
    void deleteBox()
    {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        glDeleteVertexArrays(1, &vao);
    }
    Box makeCopy()
    {
        return Box(min, max, color);
    }
    Box(const Vec3& center, const float width, const float height, const float length)
    {
        float half_width = fabs(width) * 0.5f;
        float half_height = fabs(height) * 0.5f;
        float half_length = fabs(length) * 0.5f;

        Vec3 max(half_width, half_height, half_length);
        Vec3 min = -max;
        max += center;
        min += center;        
        *this = Box(min, max);
    }

    void setShaderAndAttributes(GLuint shader)
    {
        // Setting attributes
        shader_program = shader;
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        GLsizei stride = sizeof(GLfloat) * 6; // 3 pos + 3 normal 
        GLint posAttrib = glGetAttribLocation(shader, "position");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, stride, 0);

        GLint normAttrib = glGetAttribLocation(shader, "normal");
        glEnableVertexAttribArray(normAttrib);
        glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void*)(sizeof(GLfloat) * 3));        

        glUseProgram(0);
        glBindVertexArray(0);
        num_vertices = 36;        
    }

    void changeLength(const int side_num, const float amount)
    {
        const float min_len = 0.1f;
        switch(side_num)
        {
        case 0: // Positive x
        {
            continuous_max[0] += amount;
            float diff = continuous_max[0] - continuous_min[0];
            if(diff < min_len)
            {
                continuous_max[0] += min_len - diff;
            }
        } break;
        case 1: // Negative x
        {
            continuous_min[0] -= amount;
            float diff = continuous_max[0] - continuous_min[0];
            if(diff < min_len)
            {
                continuous_min[0] -= min_len - diff;
            }
        } break;
        case 2: // Positive y
        {
            continuous_max[1] += amount;
            float diff = continuous_max[1] - continuous_min[1];
            if(diff < min_len)
            {
                continuous_max[1] += min_len - diff;
            }
        } break;
        case 3: // Negative y
        {
            continuous_min[1] -= amount;
            float diff = continuous_max[1] - continuous_min[1];
            if(diff < min_len)
            {
                continuous_min[1] -= min_len - diff;
            }
        } break;
        case 4: // Positive z
        {
            continuous_max[2] += amount;
            float diff = continuous_max[2] - continuous_min[2];
            if(diff < min_len)
            {
                continuous_max[2] += min_len - diff;
            }
        } break;
        case 5: // Negative z
        {
            continuous_min[2] -= amount;
            float diff = continuous_max[2] - continuous_min[2];
            if(diff < min_len)
            {
                continuous_min[2] -= min_len - diff;
            }
        } break;
        }

        min = snapToGrid(continuous_min);
        max = snapToGrid(continuous_max);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        constructBox();
        glBindVertexArray(0);
    }

    virtual void move(const Vec3& v)
    {
        //BBox::move(v);
        continuous_min += v;
        continuous_max += v;
        min = snapToGrid(continuous_min);
        max = snapToGrid(continuous_max);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        constructBox();
        glBindVertexArray(0);
    }

    void draw()
    {
        glUseProgram(shader_program);
        glUniform3fv(glGetUniformLocation(shader_program, "diffuse_color"), 1, (const float*)(color.data));
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    Vec3 getSideNormal(const int side_num)
    {
        switch(side_num)
        {
        case 0: // Positive x
        {
            return Vec3(1.0f, 0.0f, 0.0f);
        } break;
        case 1: // Negative x
        {
            return Vec3(-1.0f, 0.0f, 0.0f);;
        } break;
        case 2: // Positive y
        {
            return Vec3(0.0f, 1.0f, 0.0f);
        } break;
        case 3: // Negative y
        {
            return Vec3(0.0f, -1.0f, 0.0f);;
        } break;
        case 4: // Positive z
        {
            return Vec3(0.0f, 0.0f, 1.0f);
        } break;
        case 5: // Negative z
        {
            return Vec3(0.0f, 0.0f, -1.0f);;
        } break;
        }
        return Vec3();
    }

    Vec3 getColor()
    {
        return color;
    }

    void setColor(const Vec3& c)
    {
        color = c;
    }

private:
    void constructBox()
    {
        Vec3 left_vec(-1.0f, 0.0f, 0.0f);
        Vec3 right_vec(1.0f, 0.0f, 0.0f);
        Vec3 up_vec(0.0f, 1.0f, 0.0f);
        Vec3 down_vec(0.0f, -1.0f, 0.0f);
        Vec3 plus_z_vec(0.0f, 0.0f, 1.0f);
        Vec3 minus_z_vec(0.0f, 0.0f, -1.0f);

        Vec3 top_right_forward(max);
        Vec3 top_right_back(max[0], max[1], min[2]);
        Vec3 top_left_back(min[0], max[1], min[2]);
        Vec3 top_left_forward(min[0], max[1], max[2]);

        Vec3 bottom_left_back(min);
        Vec3 bottom_left_forward(min[0], min[1], max[2]);
        Vec3 bottom_right_forward(max[0], min[1], max[2]);
        Vec3 bottom_right_back(max[0], min[1], min[2]);

        std::vector<GLfloat> vertices;
        // Top
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);
        vertices.insert(vertices.end(), up_vec.data, up_vec.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), up_vec.data, up_vec.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), up_vec.data, up_vec.data+3);

        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), up_vec.data, up_vec.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), up_vec.data, up_vec.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);
        vertices.insert(vertices.end(), up_vec.data, up_vec.data+3);

        // Bottom
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), down_vec.data, down_vec.data+3);
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), down_vec.data, down_vec.data+3);
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), down_vec.data, down_vec.data+3);
            
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), down_vec.data, down_vec.data+3);
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), down_vec.data, down_vec.data+3);
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), down_vec.data, down_vec.data+3);

        // Left
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), left_vec.data, left_vec.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), left_vec.data, left_vec.data+3);
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), left_vec.data, left_vec.data+3);
            
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), left_vec.data, left_vec.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), left_vec.data, left_vec.data+3);
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);
        vertices.insert(vertices.end(), left_vec.data, left_vec.data+3);

        // Right
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), right_vec.data, right_vec.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), right_vec.data, right_vec.data+3);
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), right_vec.data, right_vec.data+3);
            
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), right_vec.data, right_vec.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), right_vec.data, right_vec.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);
        vertices.insert(vertices.end(), right_vec.data, right_vec.data+3);

        // Forward
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), plus_z_vec.data, plus_z_vec.data+3);
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), plus_z_vec.data, plus_z_vec.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);
        vertices.insert(vertices.end(), plus_z_vec.data, plus_z_vec.data+3);
            
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), plus_z_vec.data, plus_z_vec.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);
        vertices.insert(vertices.end(), plus_z_vec.data, plus_z_vec.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), plus_z_vec.data, plus_z_vec.data+3);

        // Back
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), minus_z_vec.data, minus_z_vec.data+3);
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), minus_z_vec.data, minus_z_vec.data+3);
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);        
        vertices.insert(vertices.end(), minus_z_vec.data, minus_z_vec.data+3);
            
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), minus_z_vec.data, minus_z_vec.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), minus_z_vec.data, minus_z_vec.data+3);
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);        
        vertices.insert(vertices.end(), minus_z_vec.data, minus_z_vec.data+3);

        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), &(vertices[0]), GL_STATIC_DRAW);
    }
    
    int num_vertices;
    GLuint vao, vbo, ibo;
    GLuint shader_program;
    Vec3 color;
    Vec3 continuous_min, continuous_max;
};

class BoxWireframeDrawer
{
public:
    BoxWireframeDrawer(const Mat4& proj_mat)
        :num_vertices(24)
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        shader_program = loadAndLinkShaders("shaders/boxwireframe.vs", "shaders/boxwireframe.fs");

        glUseProgram(shader_program);
        GLint proj_handle = glGetUniformLocation(shader_program, "proj_mat");
        glUniformMatrix4fv(proj_handle, 1, GL_TRUE, &(proj_mat.data[0][0]));

        glUseProgram(0);
    }

    ~BoxWireframeDrawer()
    {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ibo);
        glDeleteProgram(shader_program);
    }

    void drawWireframeOnBox(const Box& box, const Mat4& view_transform)
    {
        Vec3 offset(0.01f, 0.01f, 0.01f);
        Vec3 min = box.min - offset, max = box.max + offset;

        Vec3 top_right_forward(max);
        Vec3 top_right_back(max[0], max[1], min[2]);
        Vec3 top_left_back(min[0], max[1], min[2]);
        Vec3 top_left_forward(min[0], max[1], max[2]);
        Vec3 bottom_left_back(min);
        Vec3 bottom_left_forward(min[0], min[1], max[2]);
        Vec3 bottom_right_forward(max[0], min[1], max[2]);
        Vec3 bottom_right_back(max[0], min[1], min[2]);        

        std::vector<GLfloat> vertices;
        // Top
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);

        // Bottom
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);

        // Left
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);

        // Right
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);

        // Forward
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);

        // Back
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);                
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);
        

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), &(vertices[0]), GL_DYNAMIC_DRAW);
        glUseProgram(shader_program);
        
        // set attributes
        GLsizei stride = sizeof(GLfloat) * 3; // 3 pos
        GLint pos_attrib = glGetAttribLocation(shader_program, "position");
        glEnableVertexAttribArray(pos_attrib);
        glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, stride, 0);
        
        // set uniforms
        GLint view_handle = glGetUniformLocation(shader_program, "view_mat");
        glUniformMatrix4fv(view_handle, 1, GL_TRUE, &(view_transform.data[0][0]));
                          
        glDrawArrays(GL_LINE_STRIP, 0, num_vertices);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    void setProjTransform(const Mat4& proj_transform)
    {
        glUseProgram(shader_program);
        GLint proj_handle = glGetUniformLocation(shader_program, "proj_mat");
        glUniformMatrix4fv(proj_handle, 1, GL_TRUE, &(proj_transform.data[0][0]));
        glUseProgram(0);
    }            
private:    
    int num_vertices;
    GLuint vao, vbo, ibo;
    GLuint shader_program;
};
