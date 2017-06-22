#version 150 core

in vec2 texcoord_frag;
in vec3 normal_w_frag;

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;

uniform vec3 dir_light;

out vec4 outColor;

void main()
{
    vec4 diffuse_color = texture(diffuse_map, texcoord_frag);
	outColor = diffuse_color * dot(dir_light, normalize(normal_w_frag));
}

