#version 150 core

out vec4 outColor;

void main()
{
    vec4 frag_coord = gl_FragCoord;
    outColor = vec4(0.7f, 1, 0, 0.5);
}