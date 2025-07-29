#version 450        // GLSL 4.5

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
layout(location = 2) in vec3 aNormal;
layout(location = 3) in vec2 aUv;

// Batch input
layout(location = 4) in vec4 iModel1;
layout(location = 5) in vec4 iModel2;
layout(location = 6) in vec4 iModel3;
layout(location = 7) in vec4 iModel4;

// Batch input
layout(location = 8) in vec4 iNormalMat1;
layout(location = 9) in vec4 iNormalMat2;
layout(location = 10) in vec4 iNormalMat3;
layout(location = 11) in vec4 iNormalMat4;


layout(set = 0, binding = 0) uniform UboProjectionView {
    mat4 view;    
    mat4 projection;
    vec3 viewPos;
} uboProjectionView;

out FragInfo
{
    layout(location = 0) vec3 color;
    layout(location = 1) vec2 uv;
    layout(location = 2) vec3 normal;
    layout(location = 3) vec3 worldPos;
} fragOut;

layout(location = 4) flat out vec3 outViewPos;

void main() 
{
    mat4 iModel = mat4(iModel1, iModel2, iModel3, iModel4);
    mat4 iNormalMat = mat4(iNormalMat1, iNormalMat2, iNormalMat3, iNormalMat4);
    gl_Position = uboProjectionView.projection * uboProjectionView.view * iModel * vec4(aPos, 1.0);
    
    fragOut.color = aColor;
    fragOut.uv = aUv;
    fragOut.normal = (iNormalMat * vec4(aNormal, 1.0)).xyz;
    fragOut.worldPos = (iModel * vec4(aPos, 1.0)).xyz;

    outViewPos = uboProjectionView.viewPos;
}