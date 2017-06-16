#version 150 core

out vec4 outColor;

void main()
{
    vec4 frag_coord = gl_FragCoord;
    outColor = vec4(1, 1, 1, 0.8);
}