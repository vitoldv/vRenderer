#version 450

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 0) out vec4 FragColor; 

void main()
{
    FragColor = vec4(0.04, 0.28, 0.26, 1.0);
}