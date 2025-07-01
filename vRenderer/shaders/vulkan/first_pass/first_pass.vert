#version 450        // GLSL 4.5

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aUv;

layout(set = 0, binding = 0) uniform UboProjectionView {
    mat4 view;    
    mat4 projection;
    vec3 viewPos;
} uboProjectionView;

layout(push_constant) uniform Push {
    mat4 model;
    mat4 normalMatrix;
} push;

// Interpolated values
layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragPos;

// Flat values (mesh related)
layout(location = 4) flat out vec3 outViewPos;

void main() {
    gl_Position = uboProjectionView.projection * uboProjectionView.view * push.model * vec4(aPos, 1.0);
    
    fragColor = aColor;
    fragUv = aUv;
    fragNormal = (push.normalMatrix * vec4(aNormal, 1.0)).xyz;
    fragPos = (push.model * vec4(aPos, 1.0)).xyz;

    outViewPos = uboProjectionView.viewPos;
}