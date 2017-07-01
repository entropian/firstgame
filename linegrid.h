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

    LineGrid(const float spacing, const float height, const int num_lines,
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

        shader_program = loadAndLinkShaders("shaders/default.vs", "shaders/color.fs");
        
        glUseProgram(shader_program);
        // Uniforms
        //GLint view_handle = glGetUniformLocation(shader_program, "view_mat");
        //glUniformMatrix4fv(view_handle, 1, GL_TRUE, &(view_transform.data[0][0]));
        GLint proj_handle = glGetUniformLocation(shader_program, "proj_mat");
        glUniformMatrix4fv(proj_handle, 1, GL_TRUE, &(proj_transform.data[0][0]));
        Vec3 color(0.7f, 1.0f, 0.0f);
        glUniform3fv(glGetUniformLocation(shader_program, "color"), 1, (const float*)(color.data));

        // Attributes
        GLsizei stride = sizeof(GLfloat) * 3;
        GLint pos_attrib = glGetAttribLocation(shader_program, "position");
        glEnableVertexAttribArray(pos_attrib);
        glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, stride, 0);

        glUseProgram(0);
        glBindVertexArray(0);
    }

    ~LineGrid()
    {
        glDeleteBuffers(1, &vao);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(shader_program);        
    }

    void setViewTransform(const Mat4& view_transform)
    {
        glUseProgram(shader_program);
        GLint view_handle = glGetUniformLocation(shader_program, "view_mat");
        glUniformMatrix4fv(view_handle, 1, GL_TRUE, &(view_transform.data[0][0]));
        glUseProgram(0);
    }

    void setProjTransform(const Mat4& proj_transform)
    {
        glUseProgram(shader_program);
        GLint proj_handle = glGetUniformLocation(shader_program, "proj_mat");
        glUniformMatrix4fv(proj_handle, 1, GL_TRUE, &(proj_transform.data[0][0]));
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
    GLuint vao, vbo, shader_program;
    int num_vertices;
    float spacing;
    float height;
};
