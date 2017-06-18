#pragma once
#include <GL/glew.h>
#include <iostream>
#include <sstream>
#include <fstream>

static GLuint loadShader(const char *file_path, const GLenum shader_type)
{
	GLuint shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, &(file_path), NULL);
	glCompileShader(shader);

    GLint status;
    GLchar info_log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(status != GL_TRUE)
    {
        glGetShaderInfoLog(shader, 512, NULL, info_log);
        if(shader_type == GL_VERTEX_SHADER)
        {
            fprintf(stderr, "Vertex shader compiled incorrectly.\n");
        }else if(shader_type == GL_FRAGMENT_SHADER)
        {
            fprintf(stderr, "Fragment shader compiled incorrectly.\n");
        }
        fprintf(stderr, "%s\n", info_log);
    }
    return shader;
}

static GLuint loadAndLinkShaders(const char* vert_shader_path, const char* frag_shader_path)
{
    std::ifstream vert_fstream(vert_shader_path);
    std::stringstream buffer;
    buffer << vert_fstream.rdbuf();
    vert_fstream.close();
    std::string vert_src = buffer.str();
    const char* vert_src_cstr = vert_src.c_str();    
    GLuint vert_shader = loadShader(vert_src_cstr, GL_VERTEX_SHADER);

    std::ifstream frag_fstream(frag_shader_path);
    buffer.str("");
    buffer << frag_fstream.rdbuf();
    std::string frag_src = buffer.str();
    const char* frag_src_cstr = frag_src.c_str();
    GLuint frag_shader = loadShader(frag_src_cstr, GL_FRAGMENT_SHADER);

    GLuint shader_program = glCreateProgram();
    glAttachShader(shader_program, vert_shader);
    glAttachShader(shader_program, frag_shader);
    glBindFragDataLocation(shader_program, 0, "outColor");
    glLinkProgram(shader_program);
    glDeleteShader(frag_shader);
    glDeleteShader(vert_shader);
    return shader_program;
}
