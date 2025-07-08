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

out FragInfo
{
    layout(location = 0) vec3 color;
    layout(location = 1) vec2 uv;
    layout(location = 2) vec3 normal;
    layout(location = 3) vec3 worldPos;
} fragOut;

layout(location = 4) flat out vec3 outViewPos;

void main() {
    gl_Position = uboProjectionView.projection * uboProjectionView.view * push.model * vec4(aPos, 1.0);
    
    fragOut.color = aColor;
    fragOut.uv = aUv;
    fragOut.normal = (push.normalMatrix * vec4(aNormal, 1.0)).xyz;
    fragOut.worldPos = (push.model * vec4(aPos, 1.0)).xyz;

    outViewPos = uboProjectionView.viewPos;
}