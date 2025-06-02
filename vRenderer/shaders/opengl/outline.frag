#version 460 core

in vec3 fragColor;
in vec2 fragUv;
in vec3 fragNormal;
in vec3 fragPos;

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.04, 0.28, 0.26, 1.0);
}