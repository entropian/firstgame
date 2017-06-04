#pragma once

#include <vector>
#include <cassert>
#include "vec.h"
#include "shaders/shader.h"
#include <float.h>
#include "bbox.h"
#include <fstream>
#include <sstream>

class Box : public BBox
{
public:
    Box()
        :BBox()
    {
    }
    
    Box(const Vec3& a, const Vec3& b)
        :BBox(a, b)
    {
        // TODO: figure out the z orientation of boxes
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        constructBox();
        glBindVertexArray(0);        
        num_vertices = 36;
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

    void setAttributesToShader(GLuint shader)
    {
        // Setting attributes
        glUseProgram(shader);
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
        // TODO: clamp to minimum lengths
        const float min_len = 0.1f;
        switch(side_num)
        {
        case 0: // Positive x
        {
            max[0] += amount;
            float diff = max[0] - min[0];
            if(diff < min_len)
            {
                max[0] += min_len - diff;
            }
        } break;
        case 1: // Negative x
        {
            min[0] -= amount;
            float diff = max[0] - min[0];
            if(diff < min_len)
            {
                min[0] -= min_len - diff;
            }
        } break;
        case 2: // Positive y
        {
            max[1] += amount;
            float diff = max[1] - min[1];
            if(diff < min_len)
            {
                max[1] += min_len - diff;
            }
        } break;
        case 3: // Negative y
        {
            min[1] -= amount;
            float diff = max[1] - min[1];
            if(diff < min_len)
            {
                min[1] -= min_len - diff;
            }
        } break;
        case 4: // Positive z
        {
            max[2] += amount;
            float diff = max[2] - min[2];
            if(diff < min_len)
            {
                max[2] += min_len - diff;
            }
        } break;
        case 5: // Negative z
        {
            min[2] -= amount;
            float diff = max[2] - min[2];
            if(diff < min_len)
            {
                min[2] -= min_len - diff;
            }
        } break;
        }

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        constructBox();
        glBindVertexArray(0);
    }

    void draw()
    {
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, num_vertices);
        glBindVertexArray(0);
    }

    Vec3 getSideNormal(const int side_num)
    {
        switch(side_num)
        {
        case 0: // Positive x
        {
            return RIGHT;
        } break;
        case 1: // Negative x
        {
            return LEFT;
        } break;
        case 2: // Positive y
        {
            return UP;
        } break;
        case 3: // Negative y
        {
            return DOWN;
        } break;
        case 4: // Positive z
        {
            return PLUS_Z;
        } break;
        case 5: // Negative z
        {
            return MINUS_Z;
        } break;
        }
        return Vec3();
    }
    
private:
    void constructBox()
    {
        Vec3 top_right_forward(max);
        Vec3 top_right_back(max[0], max[1], min[2]);
        Vec3 top_left_back(min[0], max[1], min[2]);
        Vec3 top_left_forward(min[0], max[1], max[2]);

        Vec3 bottom_left_back(min);
        Vec3 bottom_left_forward(min[0], min[1], max[2]);
        Vec3 bottom_right_forward(max[0], min[1], max[2]);
        Vec3 bottom_right_back(max[0], min[1], min[2]);

        std::vector<GLfloat> vertices;
        // TODO: winding order for backside is wrong
        // Top
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);
        vertices.insert(vertices.end(), UP.data, UP.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), UP.data, UP.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), UP.data, UP.data+3);

        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), UP.data, UP.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), UP.data, UP.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);
        vertices.insert(vertices.end(), UP.data, UP.data+3);

        // Bottom
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), DOWN.data, DOWN.data+3);
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), DOWN.data, DOWN.data+3);
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), DOWN.data, DOWN.data+3);
            
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), DOWN.data, DOWN.data+3);
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), DOWN.data, DOWN.data+3);
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), DOWN.data, DOWN.data+3);

        // Left
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), LEFT.data, LEFT.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), LEFT.data, LEFT.data+3);
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), LEFT.data, LEFT.data+3);
            
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), LEFT.data, LEFT.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), LEFT.data, LEFT.data+3);
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);
        vertices.insert(vertices.end(), LEFT.data, LEFT.data+3);

        // Right
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), RIGHT.data, RIGHT.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), RIGHT.data, RIGHT.data+3);
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), RIGHT.data, RIGHT.data+3);
            
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), RIGHT.data, RIGHT.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), RIGHT.data, RIGHT.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);
        vertices.insert(vertices.end(), RIGHT.data, RIGHT.data+3);

        // Forward
        vertices.insert(vertices.end(), bottom_right_forward.data, bottom_right_forward.data+3);
        vertices.insert(vertices.end(), PLUS_Z.data, PLUS_Z.data+3);
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), PLUS_Z.data, PLUS_Z.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);
        vertices.insert(vertices.end(), PLUS_Z.data, PLUS_Z.data+3);
            
        vertices.insert(vertices.end(), bottom_left_forward.data, bottom_left_forward.data+3);
        vertices.insert(vertices.end(), PLUS_Z.data, PLUS_Z.data+3);
        vertices.insert(vertices.end(), top_right_forward.data, top_right_forward.data+3);
        vertices.insert(vertices.end(), PLUS_Z.data, PLUS_Z.data+3);
        vertices.insert(vertices.end(), top_left_forward.data, top_left_forward.data+3);
        vertices.insert(vertices.end(), PLUS_Z.data, PLUS_Z.data+3);

        // Back
        vertices.insert(vertices.end(), bottom_left_back.data, bottom_left_back.data+3);
        vertices.insert(vertices.end(), MINUS_Z.data, MINUS_Z.data+3);
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);
        vertices.insert(vertices.end(), MINUS_Z.data, MINUS_Z.data+3);
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), MINUS_Z.data, MINUS_Z.data+3);
            
        vertices.insert(vertices.end(), bottom_right_back.data, bottom_right_back.data+3);
        vertices.insert(vertices.end(), MINUS_Z.data, MINUS_Z.data+3);
        vertices.insert(vertices.end(), top_left_back.data, top_left_back.data+3);
        vertices.insert(vertices.end(), MINUS_Z.data, MINUS_Z.data+3);
        vertices.insert(vertices.end(), top_right_back.data, top_right_back.data+3);
        vertices.insert(vertices.end(), MINUS_Z.data, MINUS_Z.data+3);

        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), &(vertices[0]), GL_STATIC_DRAW);
    }
    
    int num_vertices;
    GLuint vao, vbo, ibo;
};
