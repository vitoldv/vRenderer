#version 460 core

#define MAX_LIGHT_SOURCES 10

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec3 outColor; // output a color to the fragment shader
out vec2 outTexCoord;
out vec3 outNormal;
out vec3 outFragPos;
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
    outColor = aColor;
    outNormal = normalMatrix * aNormal;
    outTexCoord = aTexCoord;

    // view space position of a fragment
    outFragPos = vec3(view * model * vec4(aPos, 1.0));
    for(int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        outLightPos[i] = vec3(view * vec4(lightPos[i], 1.0));
        outLightDir[i] = vec3(transpose(inverse(view)) * vec4(lightDir[i], 1.0));
    }
}