#pragma once
#include <GL/glew.h>
#include <vector>
#include <iostream>
#include "vec.h"
/*
  The stick part of the arrow could just be a line
  Cone part of the arrow should be drawn as a triangle fan
 */



class Manipulator
{
public:
    Manipulator(const Mat4& proj_transform)
    {
        std::vector<Vec3> cone_verts;
        generateCone(cone_verts, 1.0f, 3.0f);
        // 1 vert at the tip of the cone. 36 along the rim of the cone.
        // The first vert on the rim of the cone is counted twice 1 + 36 + 1 = 38
        num_vertices = 38; 
        std::vector<float>* float_vec = reinterpret_cast<std::vector<float>*>(&cone_verts);
        printf("cone_verts size = %d\n", cone_verts.size());
        printf("float_vec size = %d\n", (*float_vec).size());
        shader_program = loadAndLinkShaders("shaders/linegrid.vs", "shaders/linegrid.fs");

        glUseProgram(shader_program);
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * (*float_vec).size(), &((*float_vec)[0]), GL_STATIC_DRAW);

        // Uniforms
        GLint proj_handle = glGetUniformLocation(shader_program, "proj_mat");
        glUniformMatrix4fv(proj_handle, 1, GL_TRUE, &(proj_transform.data[0][0]));

        // Attributes
        GLsizei stride = sizeof(GLfloat) * 3;
        GLint pos_attrib = glGetAttribLocation(shader_program, "position");
        glEnableVertexAttribArray(pos_attrib);
        glVertexAttribPointer(pos_attrib, 3, GL_FLOAT, GL_FALSE, stride, 0);

        glBindVertexArray(0);
        glUseProgram(0);
    }

    ~Manipulator()
    {
        glDeleteBuffers(1, &vao);
        glDeleteVertexArrays(1, &vao);
        glDeleteProgram(shader_program);
    }

    void draw(const Mat4& view_transform)
    {
        glUseProgram(shader_program);
        glBindVertexArray(vao);
        GLint view_handle = glGetUniformLocation(shader_program, "view_mat");
        glUniformMatrix4fv(view_handle, 1, GL_TRUE, &(view_transform.data[0][0]));
        glDrawArrays(GL_TRIANGLE_FAN, 0, num_vertices);
        glBindVertexArray(0);
        glUseProgram(0);
    }

private:
    void generateCone(std::vector<Vec3>& vertices, const float radius, const float height)
    {
        vertices.push_back(Vec3(0.0f, height, 0.0f));
        const int num_rim_verts = 36;
        const float theta_increment = 2*M_PI / num_rim_verts;
        for(int i = 0; i < num_rim_verts + 1; i++)
        {
            float theta = i * theta_increment;
            vertices.push_back(Vec3(cosf(theta) * radius, 0.0f, sinf(theta)) * radius);
        }
    }
    int num_vertices;
    GLuint vao, vbo;
    GLuint shader_program;
};
