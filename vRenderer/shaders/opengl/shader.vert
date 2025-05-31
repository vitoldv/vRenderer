#version 460 core

#define MAX_LIGHT_SOURCES 10

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aUv;

out vec3 fragColor;
out vec2 fragUv;
out vec3 fragNormal;
out vec3 fragPos;

out vec3 outLightPos[MAX_LIGHT_SOURCES];
out vec3 outLightDir[MAX_LIGHT_SOURCES];

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

uniform vec3 lightPos[MAX_LIGHT_SOURCES];
uniform vec3 lightDir[MAX_LIGHT_SOURCES];

void main()
{
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);

    fragColor = aColor;
    fragUv = aUv;
    fragNormal = normalMatrix * aNormal;
    // view space position of a fragment
    fragPos = vec3(view * model * vec4(aPos, 1.0));

    for(int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        outLightPos[i] = vec3(view * vec4(lightPos[i], 1.0));
        outLightDir[i] = vec3(transpose(inverse(view)) * vec4(lightDir[i], 1.0));
    }
}