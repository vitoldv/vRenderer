#version 450 core
layout (location = 0) in vec3 aPos;

layout(set = 0, binding = 0) uniform UboProjectionView {
    mat4 view;    
    mat4 projection;
} uboProjectionView;

layout(push_constant) uniform Push {
    mat4 model;
    mat4 normalMatrix;
    vec3 viewPos;
} push;

layout(location = 0) out vec3 fragUv;

void main()
{
    fragUv = aPos;
    gl_Position = uboProjectionView.projection * uboProjectionView.view * push.model * vec4(aPos, 1.0);
}  