#version 150 core

in vec3 position;

uniform mat4 model_view;
uniform mat4 projection;

void main()
{
    gl_Position = model_view * vec4(position, 1.0f);
}	
