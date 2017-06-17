#pragma once
#include "box.h"
#include "ray.h"
#include <vector>
#include "vec.h"
#include <iostream>

class Track
{
public:
    Track()
    {
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
    }

    // TODO: the name impliles all the uniforms are set
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

    void addBox(Box& box)
    {
        box.setShaderAndAttributes(shader_program);
        boxes.push_back(box);
    }

    int getNumBoxes()
    {
        return boxes.size();
    }

    Box* rayIntersectTrack(int& face, float& t, const Ray& ray)
    {
        float min_t = TMAX;
        int index = -1, min_face = -1;
        for(int i = 0; i < boxes.size(); i++)
        {
            int side;
            float t = boxes[i].rayIntersect(side, ray);
            if(t < min_t)
            {
                min_t = t;
                index = i;
                min_face = side;
            }
        }
        face = min_face;
        t = min_t;
        if(index != -1)
        {
            return &(boxes[index]);
        }
        return nullptr;
    }

    // Determine if ship bbox collide with track
    // Assume all track boxes are the size of ship's bbox or bigger
    // Therefore each side of ship's bbox can touch up to 4 boxes
    // 4 * 6 sides = 24
    int bboxCollideWithTrack(Box* colliding_boxes[24], const BBox& bbox)
    {
        int count = 0;
        Vec3 center_dir(0.0f, 0.0f, 0.0f);

        Vec3 ship_center = (bbox.max - bbox.min) * 0.5f + bbox.min;
        for(int i = 0; i < boxes.size(); i++)
        {
            if(bbox.bboxIntersect(boxes[i]))
            {

                colliding_boxes[count] = &(boxes[i]);
                count++;
            }
        }
        return count;
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
        for(int i = 0; i < boxes.size(); i++)
        {
            boxes[i].draw();
        }
    }

    void print()
    {
        for(int i = 0; i < boxes.size(); i++)
        {
            boxes[i].min.print();
            boxes[i].max.print();
            std::cout << "\n";
        }
    }

    void readFromFile(const char* file_name)
    {
        std::ifstream input(file_name);
        float tmp;
        int i = 0;
        Vec3 min, max;
        while(input >> tmp)
        {
            if(i < 3)
            {
                min[i] = tmp;
            }else if(i >= 3)
            {
                max[i-3] = tmp;
            }
            i++;
            if(i == 6)
            {
                Box box(min, max);
                addBox(box);
                i = 0;
            }
        }            
        input.close();
    }

    void writeToFile(const char* file_name)
    {
        std::ofstream output(file_name);
        for(int i = 0; i < boxes.size(); i++)
        {
            output << boxes[i].min[0] << " ";
            output << boxes[i].min[1] << " ";
            output << boxes[i].min[2] << " ";
            output << boxes[i].max[0] << " ";
            output << boxes[i].max[1] << " ";
            output << boxes[i].max[2] << "\n";
        }

        output.close();
    }

    ~Track()
    {
        glDeleteProgram(shader_program);
        for(int i = 0; i < boxes.size(); i++)
        {
            boxes[i].deleteBox();
        }
    }
private:
    std::vector<Box> boxes;
    GLuint shader_program;    
};
