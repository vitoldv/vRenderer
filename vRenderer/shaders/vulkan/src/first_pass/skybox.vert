#version 450 core
layout (location = 0) in vec3 aPos;

layout(set = 0, binding = 0) uniform UboProjectionView {
    mat4 view;    
    mat4 projection;
    vec3 viewPos;
} uboProjectionView;

layout(location = 0) out vec3 fragUv;

mat4 translate(vec3 t) 
{
    return mat4(
        1.0, 0.0, 0.0, 0.0,  // Column 0
        0.0, 1.0, 0.0, 0.0,  // Column 1
        0.0, 0.0, 1.0, 0.0,  // Column 2
        t.x, t.y, t.z, 1.0   // Column 3
    );
}

void main()
{
    fragUv = aPos;
    vec4 pos = uboProjectionView.projection * uboProjectionView.view * translate(uboProjectionView.viewPos) * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  