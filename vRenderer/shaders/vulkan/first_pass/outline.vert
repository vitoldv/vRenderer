#version 450        // GLSL 4.5

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aUv;

layout(set = 0, binding = 0) uniform UboProjectionView {
    mat4 view;    
    mat4 projection;
} uboProjectionView;

layout(push_constant) uniform Push {
    mat4 model;
    mat4 normalMatrix;
    vec3 viewPos;
} push;

// Interpolated values
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPos;

// Flat values (mesh related)
layout(location = 4) flat out vec3 outViewPos;

#define percent 0.04

void main()
{
    vec3 normal = normalize(aNormal);
    vec4 newPos = vec4(aPos + normal * percent, 1.0);
    gl_Position = uboProjectionView.projection * uboProjectionView.view * push.model * newPos;
    fragColor = aColor;
    fragUv = aUv;
    fragNormal = mat3(push.normalMatrix) * aNormal;
    // view space position of a fragment
    fragPos = vec3(uboProjectionView.view * push.model * newPos);
}