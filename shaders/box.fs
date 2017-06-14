#version 150 core

in vec3 normal_w_frag;

uniform vec3 dir_light_1;
uniform vec3 dir_light_2;
uniform vec3 diffuse_color;

out vec4 outColor;

void main()
{
    //vec4 diffuse_color = vec4(0.7, 0.7, 0.7, 1.0);
    float cos_theta_1 = clamp(dot(dir_light_1, normal_w_frag), 0, 1);
    float cos_theta_2 = clamp(dot(dir_light_2, normal_w_frag), 0, 1);
	outColor = vec4(diffuse_color * (cos_theta_1 + cos_theta_2), 1);
    //outColor = vec4(1, 1, 1, 1);
}
