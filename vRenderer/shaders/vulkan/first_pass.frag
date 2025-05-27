#version 450

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) flat in uint fragTextured;

layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 2, binding = 0) uniform sampler2D specularMap;

layout(location = 0) out vec4 outColor;     // final output color

void main()
{
    if(fragTextured != 0)
    {
        outColor = texture(diffuseMap, fragUv);
    }
    else
    {
        outColor = vec4(vec3(1.0f), 1.0f);
    }
}