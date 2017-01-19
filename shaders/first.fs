#version 150 core

in vec2 texcoord_frag;

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;

out vec4 outColor;

void main()
{
    vec4 diffuse_color = texture(diffuse_map, texcoord_frag);
	outColor = diffuse_color;
}

