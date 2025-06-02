#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aUv;

out vec3 fragColor;
out vec2 fragUv;
out vec3 fragNormal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

#define percent 0.04

void main()
{
    vec3 normal = normalize(aNormal);
    vec4 newPos = vec4(aPos + normal * percent, 1.0);
    gl_Position = projection * view * model * newPos;

    fragColor = aColor;
    fragUv = aUv;
    fragNormal = normalMatrix * aNormal;
    // view space position of a fragment
    fragPos = vec3(view * model * newPos);
}