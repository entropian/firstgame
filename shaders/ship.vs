#version 150 core

in vec3 position;
in vec3 normal;
in vec2 texcoord;

uniform mat4 model_view_mat;
uniform mat4 normal_mat;
uniform mat4 proj_mat;

out vec2 texcoord_frag;
out vec3 normal_w_frag;

void main()
{
    texcoord_frag = texcoord;
    normal_w_frag = (normal_mat * vec4(normal, 0)).xyz;
    gl_Position = proj_mat * model_view_mat * vec4(position, 1.0f);
}	
