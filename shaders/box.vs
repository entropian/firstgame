#version 150 core

in vec3 position;
in vec3 normal;

uniform mat4 model_view_mat;
uniform mat4 normal_mat;
uniform mat4 proj_mat;

out vec3 normal_w_frag;

void main()
{
    normal_w_frag = (normal_mat * vec4(normal, 0.0)).rgb;
    gl_Position = proj_mat * model_view_mat * vec4(position, 1.0f);
}	
