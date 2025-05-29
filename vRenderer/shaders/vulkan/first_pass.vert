#version 450        // GLSL 4.5

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 col;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(set = 0, binding = 0) uniform UboProjectionView {
    mat4 projection;
    mat4 view;    
} uboProjectionView;

// LEFT FOR REFERENCE ON DYNAMIC UNIFORM BUFFERS
// layout(set = 0, binding = 1) uniform UboModel {
//     mat4 model;  
// } uboModel;

layout(push_constant) uniform PushModel {
    mat4 model;
    mat4 normalMatrix;
    vec3 viewPos;
    uint textured;
} pushModel;

layout(location = 0) out vec3 fragCol;
layout(location = 1) out vec2 fragUv;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) flat out uint fragTextured;
layout(location = 4) flat out vec3 outViewPos;
layout(location = 5) out vec3 outFragPos;

void main() {
    gl_Position = uboProjectionView.projection * uboProjectionView.view * pushModel.model * vec4(pos, 1.0);
    fragCol = col;
    fragUv = uv;
    fragNormal = (pushModel.normalMatrix * vec4(normal, 1.0)).xyz;
    fragTextured = pushModel.textured;
    outViewPos = pushModel.viewPos;
    outFragPos = (pushModel.model * vec4(pos, 1.0)).xyz;
}