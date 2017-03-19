#pragma once
#include <GL/glew.h>
#include <vector>
#include <cassert>
#include "vec.h"
#include "shaders/shader.h"
#include <float.h>
#include "bbox.h"

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

        // Shaders
        std::ifstream vert_fstream("shaders/box.vs");
        std::stringstream buffer;
        buffer << vert_fstream.rdbuf();
        vert_fstream.close();
        std::string vert_src = buffer.str();
        const char* vert_src_cstr = vert_src.c_str();    
        GLuint vert_shader = loadShader(vert_src_cstr, GL_VERTEX_SHADER);

        std::ifstream frag_fstream("shaders/box.fs");
        buffer.str("");
        buffer << frag_fstream.rdbuf();
        std::string frag_src = buffer.str();
        const char* frag_src_cstr = frag_src.c_str();
        GLuint frag_shader = loadShader(frag_src_cstr, GL_FRAGMENT_SHADER);

        shader_program = glCreateProgram();
        glAttachShader(shader_program, vert_shader);
        glAttachShader(shader_program, frag_shader);
        glBindFragDataLocation(shader_program, 0, "outColor");
        glLinkProgram(shader_program);
        glUseProgram(shader_program);
        glDeleteShader(frag_shader);
        glDeleteShader(vert_shader);

        // Setting attributes
        GLsizei stride = sizeof(GLfloat) * 6; // 3 pos + 3 normal 
        GLint posAttrib = glGetAttribLocation(shader_program, "position");
        glEnableVertexAttribArray(posAttrib);
        glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, stride, 0);

        GLint normAttrib = glGetAttribLocation(shader_program, "normal");
        glEnableVertexAttribArray(normAttrib);
        glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void*)(sizeof(GLfloat) * 3));        

        glUseProgram(0);
        glBindVertexArray(0);
        num_vertices = 36;
    }

    void setUniforms(const Mat4& view_transform, const Mat4& normal_transform,
                     const Mat4& proj_transform, const Vec3& dir_light_1,
                     const Vec3& dir_light_2)
    {
        glUseProgram(shader_program);
        GLint view_handle = glGetUniformLocation(shader_program, "view_mat");
        glUniformMatrix4fv(view_handle, 1, GL_TRUE, &(view_transform.data[0][0]));
        GLint proj_handle = glGetUniformLocation(shader_program, "proj_mat");
        glUniformMatrix4fv(proj_handle, 1, GL_TRUE, &(proj_transform.data[0][0]));
        GLint normal_transform_handle = glGetUniformLocation(shader_program, "normal_mat");
        glUniformMatrix4fv(normal_transform_handle, 1, GL_TRUE, &(normal_transform.data[0][0]));
    
        GLint dir_light_1_handle = glGetUniformLocation(shader_program, "dir_light_1");
        glUniform3fv(dir_light_1_handle, 1, (const GLfloat*)(dir_light_1.data));
        GLint dir_light_2_handle = glGetUniformLocation(shader_program, "dir_light_2");
        glUniform3fv(dir_light_2_handle, 1, (const GLfloat*)(dir_light_2.data));
        glUseProgram(0);
    }

    void setViewTransform(const Mat4& view_transform)
    {
        glUseProgram(shader_program);
        GLint view_handle = glGetUniformLocation(shader_program, "view_mat");
        glUniformMatrix4fv(view_handle, 1, GL_TRUE, &(view_transform.data[0][0]));
        glUseProgram(0);
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
        glUseProgram(shader_program);
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
    GLuint shader_program;
};
