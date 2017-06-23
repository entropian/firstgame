#pragma once
#include <GL/glew.h>
#include <vector>
#include <iostream>
#include <assert.h>
#include "vec.h"
#include "box.h"
#include "selected.h"
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
        const float cone_radius = 0.25f;
        const float cone_height = 1.25f;
        generateCone(cone_verts, cone_radius, cone_height);
        const float handle_length = 2.0f;
        num_vert_per_cone = cone_verts.size();
        // Y cone
        for(int i = 0; i < num_vert_per_cone; i++)
        {
            cone_verts[i][1] += handle_length;
        }
        cone_verts.push_back(Vec3(0.0f, 0.0f, 0.0f));
        cone_verts.push_back(Vec3(0.0f, handle_length, 0.0f));
        num_vert_per_arrow = num_vert_per_cone + 2;
        
        // X cone
        Mat4 rotation = Mat4::makeZRotation(-90.0f);
        for(int i = 0; i < num_vert_per_cone; i++)
        {            
            cone_verts.push_back(Vec3(rotation * Vec4(cone_verts[i], 1.0f)));
        }
        cone_verts.push_back(Vec3(0.0f, 0.0f, 0.0f));
        cone_verts.push_back(Vec3(handle_length, 0.0f, 0.0f));

        // Z cone
        rotation = Mat4::makeXRotation(90.0f);
        for(int i = 0; i < num_vert_per_cone; i++)
        {            
            cone_verts.push_back(Vec3(rotation * Vec4(cone_verts[i], 1.0f)));
        }
        cone_verts.push_back(Vec3(0.0f, 0.0f, 0.0f));
        cone_verts.push_back(Vec3(0.0f, 0.0f, handle_length));        
        // 1 vert at the tip of the cone. 36 along the rim of the cone.
        // The first vert on the rim of the cone is counted twice 1 + 36 + 1 = 38
        num_vertices = 38;
        std::vector<float>* float_vec = reinterpret_cast<std::vector<float>*>(&cone_verts);
        shader_program = loadAndLinkShaders("shaders/model.vs", "shaders/color.fs");

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

        // Construct bounding boxes
        // X arrow
        bboxes[0].min = Vec3(0.0f, -cone_radius, -cone_radius);
        bboxes[0].max = Vec3(cone_height + handle_length, cone_radius, cone_radius);
        // Y arrow
        bboxes[1].min = Vec3(-cone_radius, 0.0f, -cone_radius);
        bboxes[1].max = Vec3(cone_radius, cone_height + handle_length, cone_radius);
        // Z arrow
        bboxes[2].min = Vec3(-cone_radius, -cone_radius, 0.0f);
        bboxes[2].max = Vec3(cone_radius, cone_radius, cone_height + handle_length);

        for(int i = 0; i < 3; i++)
        {
            arrow_bboxes_at_origin[i] = bboxes[i];
        }
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
        GLint model_handle = glGetUniformLocation(shader_program, "model_mat");
        glUniformMatrix4fv(model_handle, 1, GL_TRUE, &(model_transform.data[0][0]));
        Vec3 color(0.0f, 0.0f, 1.0f);
        glUniform3fv(glGetUniformLocation(shader_program, "color"), 1, (const float*)(color.data));
        // Draw cone
        glDrawArrays(GL_TRIANGLE_FAN, 0, num_vert_per_cone);
        // Draw handle
        glDrawArrays(GL_LINES, num_vert_per_cone, 2);
        color = Vec3(1.0f, 0.0f, 0.0f);
        glUniform3fv(glGetUniformLocation(shader_program, "color"), 1, (const float*)(color.data));        
        glDrawArrays(GL_TRIANGLE_FAN, num_vert_per_arrow, num_vert_per_cone);
        glDrawArrays(GL_LINES, num_vert_per_arrow + num_vert_per_cone, 2);
        color = Vec3(0.0f, 1.0f, 0.0f);
        glUniform3fv(glGetUniformLocation(shader_program, "color"), 1, (const float*)(color.data));        
        glDrawArrays(GL_TRIANGLE_FAN, 2 * num_vert_per_arrow, num_vert_per_cone);
        glDrawArrays(GL_LINES, 2 * num_vert_per_arrow + num_vert_per_cone, 2);
        glBindVertexArray(0);
        glUseProgram(0);
    }

    // Test whether ray hits any of the bounding boxes
    // If yes, store which arrow was hit in hit_arrow
    bool rayIntersect(float& t, const Ray& ray)
    {
        int face;
        float x_arrow_t = bboxes[0].rayIntersect(face, ray);
        float y_arrow_t = bboxes[1].rayIntersect(face, ray);
        float z_arrow_t = bboxes[2].rayIntersect(face, ray);
        if(x_arrow_t < y_arrow_t && x_arrow_t < z_arrow_t)
        {
            t = x_arrow_t;
            hit_arrow = 0;
            return true;
        }else
        {
            if(y_arrow_t < z_arrow_t)
            {
                t = y_arrow_t;
                hit_arrow = 1;
                return true;                
            }else if(z_arrow_t < y_arrow_t)
            {
                t = z_arrow_t;
                hit_arrow = 2;
                return true;
            }
        }
        hit_arrow = -1;
        return false;
    }

    void attachToBox(const Box& box)
    {
        Vec3 box_center = box.min + (box.max - box.min) * 0.5f;
        model_transform = Mat4::makeTranslation(box_center);
        for(int i = 0; i < 3; i++)
        {
            bboxes[i].min = arrow_bboxes_at_origin[i].min + box_center;
            bboxes[i].max = arrow_bboxes_at_origin[i].max + box_center;
        }
    }

    void moveTo(const Vec3& v)
    {
        model_transform = Mat4::makeTranslation(v);
        for(int i = 0; i < 3; i++)
        {
            bboxes[i].min = arrow_bboxes_at_origin[i].min + v;
            bboxes[i].max = arrow_bboxes_at_origin[i].max + v;
        }
    }

    void moveBox(Box& box, const Vec3& v)
    {
        assert(hit_arrow != -1);        
        Vec3 movement_axis(model_transform.getColumn(hit_arrow));
        float amount = dot(v, movement_axis) * 10.0f;
        box.move(movement_axis * amount);        
    }

    void moveSelected(Selected& selected, const Vec3& v)
    {
        assert(hit_arrow != -1);        
        Vec3 movement_axis(model_transform.getColumn(hit_arrow));
        float amount = dot(v, movement_axis) * 10.0f;
        selected.move(movement_axis * amount);        
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
            vertices.push_back(Vec3(cosf(theta) * radius, 0.0f, sinf(theta) * radius));
        }
    }
    Mat4 model_transform;
    BBox bboxes[3];
    int num_vertices;
    int num_vert_per_cone, num_vert_per_arrow;
    int hit_arrow;
    GLuint vao, vbo;
    GLuint shader_program;
    static BBox arrow_bboxes_at_origin[3];
};

BBox Manipulator::arrow_bboxes_at_origin[3];
