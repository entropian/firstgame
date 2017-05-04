#pragma once
#include <vector>
#include <iostream>
#include "shaders/shader.h"

class LineGrid
{
public:
    LineGrid()
    {
    }

    LineGrid(const float spacing, const float height, const int num_lines, const Mat4& view_transform,
        const Mat4& proj_transform)
    {
        this->spacing = spacing;
        this->height = height;

        // Each line is represented by two unique points
        float x_max = num_lines * spacing;
        float x_min = -x_max;
        float z_max = x_max;
        float z_min = x_min;
        std::vector<float> points;

        points.push_back(x_min);
        points.push_back(height);
        points.push_back(0.0f);

        points.push_back(x_max);
        points.push_back(height);
        points.push_back(0.0f);

        points.push_back(0.0f);
        points.push_back(height);
        points.push_back(z_min);

        points.push_back(0.0f);
        points.push_back(height);
        points.push_back(z_max);
        for(int i = 1; i <= num_lines; i++)
        {
            float dist_from_origin = (float)i * spacing;
            // Lines parallel to x axis
            points.push_back(x_min);
            points.push_back(height);
            points.push_back(-dist_from_origin);

            points.push_back(x_max);
            points.push_back(height);
            points.push_back(-dist_from_origin);

            points.push_back(x_min);
            points.push_back(height);
            points.push_back(dist_from_origin);

            points.push_back(x_max);
            points.push_back(height);
            points.push_back(dist_from_origin);
            
            // Lines parallen to z axis
            points.push_back(-dist_from_origin);
            points.push_back(height);
            points.push_back(z_min);

            points.push_back(-dist_from_origin);
            points.push_back(height);
            points.push_back(z_max);

            points.push_back(dist_from_origin);
            points.push_back(height);
            points.push_back(z_min);

            points.push_back(dist_from_origin);
            points.push_back(height);
            points.push_back(z_max);
        }
        num_vertices = points.size();
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * points.size(), &(points[0]), GL_STATIC_DRAW);



        // Shaders
        std::ifstream vert_fstream("shaders/linegrid.vs");
        std::stringstream buffer;
        buffer << vert_fstream.rdbuf();
        vert_fstream.close();
        std::string vert_src = buffer.str();
        const char* vert_src_cstr = vert_src.c_str();    
        GLuint vert_shader = loadShader(vert_src_cstr, GL_VERTEX_SHADER);

        std::ifstream frag_fstream("shaders/linegrid.fs");
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
        glDeleteShader(frag_shader);
        glDeleteShader(vert_shader);

        
        glUseProgram(shader_program);
        // Uniforms
        GLint view_handle = glGetUniformLocation(shader_program, "view_mat");
        glUniformMatrix4fv(view_handle, 1, GL_TRUE, &(view_transform.data[0][0]));
        GLint proj_handle = glGetUniformLocation(shader_program, "proj_mat");
        glUniformMatrix4fv(proj_handle, 1, GL_TRUE, &(proj_transform.data[0][0]));

        // Attributes
        GLsizei stride = sizeof(GLfloat) * 3;
        GLint pos_attrib = glGetAttribLocation(shader_program, "position");
        glEnableVertexAttribArray(pos_attrib);
        glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, stride, 0);

        glUseProgram(0);
        glBindVertexArray(0);
    }

    void setViewTransform(const Mat4& view_transform)
    {
        glUseProgram(shader_program);
        GLint view_handle = glGetUniformLocation(shader_program, "view_mat");
        glUniformMatrix4fv(view_handle, 1, GL_TRUE, &(view_transform.data[0][0]));
        glUseProgram(0);
    }

    void draw()
    {
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, num_vertices);
        glBindVertexArray(0);
        glUseProgram(0);
    }

private:
    GLuint vao, vbo, ibo, shader_program;
    int num_vertices;
    float spacing;
    float height;
};
