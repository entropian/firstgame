#version 150 core

in vec3 normal_w_frag;

uniform vec3 dir_light;
uniform vec3 diffuse_color;

out vec4 outColor;

void main()
{
    //vec4 diffuse_color = vec4(0.7, 0.7, 0.7, 1.0);
    vec3 amb_contrib = diffuse_color * 0.2;
    float cos_theta = clamp(dot(dir_light, normal_w_frag), 0, 1);
    vec3 diff_contrib;
    if(cos_theta < 0.0)
    {
        cos_theta = 0;
    }
    outColor = vec4(diffuse_color * cos_theta + amb_contrib, 1);
}
