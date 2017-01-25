#include "ship.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include "texture.h"
#include "shaders/shader.h"
#define OBJ_LOADER_IMPLEMENTATION
#include "objloader/objloader.h"

Ship::Ship()
{
    std::string model_base_path("models/");    
    std::string model_file_path(model_base_path + "Ship2.obj");
    // Vertex data
    OBJShape *obj_shapes;
    OBJMaterial *obj_materials;
    int num_shapes, num_mat;
    loadOBJ(&obj_shapes, &obj_materials, &num_shapes, &num_mat, model_file_path.c_str());

    OBJShape *ship_obj = &(obj_shapes[0]);
    std::vector<GLfloat> ship_vert_data;
    std::cout << "num_positions: " << ship_obj->num_positions << std::endl;
    std::cout << "num_normals: " << ship_obj->num_normals << std::endl;
    std::cout << "num_texcoords: " << ship_obj->num_texcoords << std::endl;
    ship_vert_data.reserve(ship_obj->num_positions + ship_obj->num_normals + ship_obj->num_texcoords);
    for(int i = 0; i < ship_obj->num_positions/3; i++)
    {
        ship_vert_data.push_back(ship_obj->positions[i*3]);
        ship_vert_data.push_back(ship_obj->positions[i*3 + 1]);
        ship_vert_data.push_back(ship_obj->positions[i*3 + 2]);
        ship_vert_data.push_back(ship_obj->normals[i*3]);
        ship_vert_data.push_back(ship_obj->normals[i*3 + 1]);
        ship_vert_data.push_back(ship_obj->normals[i*3 + 2]);
        ship_vert_data.push_back(ship_obj->texcoords[i*2]);
        ship_vert_data.push_back(-ship_obj->texcoords[i*2 + 1]);
    }
        
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * ship_vert_data.size(),
                 &(ship_vert_data[0]), GL_STATIC_DRAW);

    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint) * ship_obj->num_indices, ship_obj->indices,
                 GL_STATIC_DRAW);
    num_indices = ship_obj->num_indices;


    // Textures
    Texture ship_diffuse_tex("models/Ship2_diffuse.png");
    Texture ship_normal_tex("models/Ship2_Normal.png");

    glGenTextures(1, &diffuse_handle);
    glGenTextures(1, &normal_handle);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ship_diffuse_tex.width, ship_diffuse_tex.height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, ship_diffuse_tex.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal_handle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ship_normal_tex.width, ship_normal_tex.height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, ship_normal_tex.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Shaders
    std::ifstream vert_fstream("shaders/ship.vs");
    std::stringstream buffer;
    buffer << vert_fstream.rdbuf();
    vert_fstream.close();
    std::string vert_src = buffer.str();
    const char* vert_src_cstr = vert_src.c_str();    
    GLuint vert_shader = loadShader(vert_src_cstr, GL_VERTEX_SHADER);

    std::ifstream frag_fstream("shaders/ship.fs");
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
    GLsizei stride = sizeof(GLfloat) * 8; // 3 pos + 3 pos + 2 texcoord 
    GLint posAttrib = glGetAttribLocation(shader_program, "position");
    glEnableVertexAttribArray(posAttrib);
    glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, stride, 0);

    GLint normAttrib = glGetAttribLocation(shader_program, "normal");
    glEnableVertexAttribArray(normAttrib);
    glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, stride, (const void*)(sizeof(GLfloat) * 3));

    GLint texcoordAttrib = glGetAttribLocation(shader_program, "texcoord");
    glEnableVertexAttribArray(texcoordAttrib);
    glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE, stride, (const void*)(sizeof(GLfloat) * 6));

    glUseProgram(0);
    glBindVertexArray(0);
        
    for (int i = 0; i < num_shapes; ++i)
    {
        OBJShape_destroy(&(obj_shapes[i]));
    }
}

Ship::~Ship()
{
    glDeleteProgram(shader_program);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteVertexArrays(1, &vao);
}

void Ship::setUniforms(const Mat4& transform, const Mat4& normal_transform,
                     const Mat4 proj_transform, const Vec3& dir_light_1, const Vec3& dir_light_2)
{
    glUseProgram(shader_program);
    GLint model_view_handle = glGetUniformLocation(shader_program, "model_view_mat");
    glUniformMatrix4fv(model_view_handle, 1, GL_TRUE, &(transform.data[0][0]));
    GLint proj_handle = glGetUniformLocation(shader_program, "proj_mat");
    glUniformMatrix4fv(proj_handle, 1, GL_TRUE, &(proj_transform.data[0][0]));
    GLint normal_transform_handle = glGetUniformLocation(shader_program, "normal_mat");
    glUniformMatrix4fv(normal_transform_handle, 1, GL_TRUE, &(normal_transform.data[0][0]));
    
    GLint diffuse_map_handle = glGetUniformLocation(shader_program, "diffuse_map");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_handle);
    glUniform1i(diffuse_map_handle, 0);
    GLint dir_light_1_handle = glGetUniformLocation(shader_program, "dir_light_1");
    glUniform3fv(dir_light_1_handle, 1, (const GLfloat*)(dir_light_1.data));
    GLint dir_light_2_handle = glGetUniformLocation(shader_program, "dir_light_2");
    glUniform3fv(dir_light_2_handle, 1, (const GLfloat*)(dir_light_2.data));
    glUseProgram(0);
}

void Ship::draw()
{
    glBindVertexArray(vao);
    glUseProgram(shader_program);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}
