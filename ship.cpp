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
#include "mat.h"


Ship::Ship()
{
    std::string model_base_path("models/");    
    std::string model_file_path(model_base_path + "Ship2.obj");
    // Vertex data
    OBJShape *obj_shapes;
    OBJMaterial *obj_materials;
    int num_shapes, num_mat;
    loadOBJ(&obj_shapes, &obj_materials, &num_shapes, &num_mat, model_file_path.c_str());

    bbox = BBox(Vec3(FLT_MAX, FLT_MAX, FLT_MAX), Vec3(-TMAX, -TMAX, -TMAX));

    OBJShape *ship_obj = &(obj_shapes[0]);
    std::vector<GLfloat> ship_vert_data;
    std::cout << "num_positions: " << ship_obj->num_positions << std::endl;
    std::cout << "num_normals: " << ship_obj->num_normals << std::endl;
    std::cout << "num_texcoords: " << ship_obj->num_texcoords << std::endl;
    ship_vert_data.reserve(ship_obj->num_positions + ship_obj->num_normals + ship_obj->num_texcoords);
    const float scale = 0.2f;
    for(int i = 0; i < ship_obj->num_positions/3; i++)
    {
        Vec3 scaled_pos(ship_obj->positions[i*3] * scale, ship_obj->positions[i*3 + 1] * scale,
            ship_obj->positions[i*3 + 2] * scale);
        ship_vert_data.push_back(scaled_pos[0]);
        ship_vert_data.push_back(scaled_pos[1]);
        ship_vert_data.push_back(scaled_pos[2]);
        ship_vert_data.push_back(ship_obj->normals[i*3]);
        ship_vert_data.push_back(ship_obj->normals[i*3 + 1]);
        ship_vert_data.push_back(ship_obj->normals[i*3 + 2]);
        ship_vert_data.push_back(ship_obj->texcoords[i*2]);
        ship_vert_data.push_back(-ship_obj->texcoords[i*2 + 1]);
        bbox.enlargeTo(scaled_pos);
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

    glGenTextures(1, &diffuse_map);
    glGenTextures(1, &normal_map);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_map);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ship_diffuse_tex.width, ship_diffuse_tex.height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, ship_diffuse_tex.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normal_map);
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

    for (int i = 0; i < num_shapes; ++i)
    {
        OBJShape_destroy(&(obj_shapes[i]));
    }
    // Get shader uniform handles
    u_model_mat = glGetUniformLocation(shader_program, "model_mat");
    u_view_mat = glGetUniformLocation(shader_program, "view_mat");
    u_normal_mat = glGetUniformLocation(shader_program, "normal_mat");

    // Center the ship
    Vec3 bbox_center = (bbox.max - bbox.min) * 0.5 + bbox.min;
    Mat4 translation = Mat4::makeTranslation(-bbox_center);
    // Initial rotation for pointing the ship at -z
    Mat4 rotation = Mat4::makeYRotation(180.0f) * Mat4::makeXRotation(90.0f);
    transform = rotation * translation;
    bbox.transform(transform);
    glUniformMatrix4fv(u_model_mat, 1, GL_TRUE, &(transform.data[0][0]));

    glUseProgram(0);
    glBindVertexArray(0);
}

Ship::~Ship()
{
    glDeleteProgram(shader_program);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteVertexArrays(1, &vao);
}

void Ship::setStaticUniforms(const Mat4& proj_transform, const Vec3& dir_light_1, const Vec3& dir_light_2)
{
    glUseProgram(shader_program);
    GLint u_proj_mat = glGetUniformLocation(shader_program, "proj_mat");
    glUniformMatrix4fv(u_proj_mat, 1, GL_TRUE, &(proj_transform.data[0][0]));
    
    GLint u_diffuse_map = glGetUniformLocation(shader_program, "diffuse_map");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuse_map);
    glUniform1i(u_diffuse_map, 0);
    GLint u_dir_light_1 = glGetUniformLocation(shader_program, "dir_light_1");
    glUniform3fv(u_dir_light_1, 1, (const GLfloat*)(dir_light_1.data));
    GLint u_dir_light_2 = glGetUniformLocation(shader_program, "dir_light_2");
    glUniform3fv(u_dir_light_2, 1, (const GLfloat*)(dir_light_2.data));
    glUseProgram(0);
}

// Called after ship velocity and position are resolved
void Ship::updateDynamicUniforms(const Mat4& view_transform)
{
    Mat4 normal_transform = (view_transform * this->transform.inverse()).transpose();
    glUseProgram(shader_program);
    glUniformMatrix4fv(u_normal_mat, 1, GL_TRUE, &(normal_transform.data[0][0]));
    glUniformMatrix4fv(u_model_mat, 1, GL_TRUE, &(transform.data[0][0]));
    glUniformMatrix4fv(u_view_mat, 1, GL_TRUE, &(view_transform.data[0][0]));
    glUseProgram(0);
}

bool Ship::bboxCollide(const BBox& bbox) const
{
    return bbox.bboxIntersect(bbox);
}

void Ship::setViewTransform(const Mat4& view_transform)
{
    glUseProgram(shader_program);
    GLint view_handle = glGetUniformLocation(shader_program, "view_mat");
    glUniformMatrix4fv(view_handle, 1, GL_TRUE, &(view_transform.data[0][0]));
    glUseProgram(0);
}

BBox Ship::getBBox()
{
    return bbox;
}

void Ship::draw()
{
    glBindVertexArray(vao);
    glUseProgram(shader_program);
    glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Ship::updatePosAndVelocity(const float dt, Track& track)
{
    Vec3 dp;
    BBox new_bbox(bbox);
    if(velocity[0] != 0.0f || velocity[1] != 0.0f || velocity[2] != 0.0f)
    {
        dp = velocity * dt;
        new_bbox.min += dp;
        new_bbox.max += dp;
    }

    int collided = track.bboxCollideWithTrack(colliding_boxes, new_bbox);
    for(int i = 0; collided > 0 && i < 3; i++)
    {
        int hit_dir = -1;
        float overlap_time = FLT_MAX;
        for(int i = 0; i < collided; i++)
        {
            Box* track_box = colliding_boxes[i];
            int tmp_hit_dir;
            float tmp_overlap_time = new_bbox.calcOverlapTime(tmp_hit_dir, track_box, velocity);
            
            if(tmp_overlap_time < overlap_time)
            {
                overlap_time = tmp_overlap_time;
                hit_dir = tmp_hit_dir;
            }
        }
        if(overlap_time < FLT_MAX)
        {
            printf("dt %f\n", dt);
            printf("overlap_time %f\n", overlap_time);
            dp = velocity * (dt - overlap_time - (dt * 0.05f));
            velocity[hit_dir] = 0.0f;
            dp += velocity * (overlap_time + (dt * 0.05f));
        }
        new_bbox.min = bbox.min + dp;
        new_bbox.max = bbox.max + dp;
        collided = track.bboxCollideWithTrack(colliding_boxes, new_bbox);
    }
    bbox.min += dp;
    bbox.max += dp;
    Mat4 displacement = Mat4::makeTranslation(dp);
    transform = displacement * transform;
}

// TODO: make accleration time based instead of frame based
void Ship::calcVelocity(int accel_states[3])
{
    // TODO: velocity oscillates around 0    
    // Z velocity
    // Ship going "forward"
    if(velocity[2] < 0.0f)
    {
        if(accel_states[2] == -1)
        {
            // Accelerating in the same direction as ship motion
            velocity[2] = -MAX_Z_VELOCITY;
        }else if(accel_states[2] == 1)
        {
            // Acclerating in the opposite direction as ship motion
            velocity[2] += 5.0f;
        }else
        {
            // Slowing down to 0
            velocity[2] += 3.0f;
        }
    }else if(velocity[2] > 0.0f)
    {
        if(accel_states[2] == -1)
        {
            // Accelerating in the opposite direction as ship motion
            velocity[2] -= 5.0;
        }else if(accel_states[2] == 1)
        {
            // Acclerating in the same direction as ship motion
            velocity[2] = MAX_Z_VELOCITY;
        }else
        {
            // Slowing down to 0
            velocity[2] -= 3.0f;
        }        
    }else if(velocity[2] == 0)
    {
        if(accel_states[2] == -1)
        {
            // Forward
            velocity[2] = -MAX_Z_VELOCITY;
        }else if(accel_states[2] == 1)
        {
            // Backward
            velocity[2] = MAX_Z_VELOCITY;
        }
    }

    // X velocity
    if(accel_states[0] == -1)
    {
        velocity[0] = -MAX_X_VELOCITY;
    }else if(accel_states[0] == 1)
    {
        velocity[0] = MAX_X_VELOCITY;
    }else if(accel_states[0] == 0)
    {
        velocity[0] = 0;
    }

    // Y velocity
    if(accel_states[1] == -1)
    {
        velocity[1] -= 2.0f;
        if(velocity[1] < MAX_Y_DOWNWARD_VELOCITY)
        {
            velocity[1] = MAX_Y_DOWNWARD_VELOCITY;
        }
    }else if(accel_states[1] == 1)
    {
        velocity[1] = Y_JUMP_VELOCITY;
    }
}

void Ship::move(const Vec3& v)
{
    Mat4 translation = Mat4::makeTranslation(v);
    transform = translation * transform;
    bbox.min += v;
    bbox.max += v;
}
