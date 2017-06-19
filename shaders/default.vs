#version 150 core

in vec3 position;

uniform mat4 view_mat;
uniform mat4 proj_mat;

void main()
{
    gl_Position = proj_mat * view_mat * vec4(position, 1.0f);
}