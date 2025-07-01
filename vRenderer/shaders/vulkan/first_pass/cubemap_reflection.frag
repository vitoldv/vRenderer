#version 450 core

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) flat in vec3 outViewPos;
layout (set = 1, binding = 0) uniform samplerCube cubemapSampler;
layout(location = 0) out vec4 FragColor;

void main()
{
    vec3 viewDir = normalize(fragPos - outViewPos);
    vec3 reflection = reflect(viewDir, fragNormal);
    FragColor = texture(cubemapSampler, reflection);
}