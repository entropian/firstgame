#version 150 core

in vec3 position;
in vec3 normal;
in vec2 texcoord;

uniform mat4 model_view;
uniform mat4 projection;

out vec2 texcoord_frag;

void main()
{
    texcoord_frag = texcoord;
    gl_Position = projection * model_view * vec4(position, 1.0f);
}	
