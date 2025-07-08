#version 450 core

layout (location = 0) in vec3 aPos;

out FragInfo
{
    layout(location = 0) vec2 uv;
    layout(location = 1) vec3 normal;
    layout(location = 2) vec3 worldPos;
} fragOut; 

layout(location = 3) out vec3 outViewPos;

layout(set = 0, binding = 0) uniform UboProjectionView {
    mat4 view;    
    mat4 projection;
    vec3 viewPos;
} uboProjectionView;

float normalize_range(float value, float min_v, float max_v)
{
    return (value - min_v)/(max_v - min_v);
}

vec2 normalize_range_vec2(vec2 vector, float min_v, float max_v)
{
    vec2 res = vec2(vector.x - min_v, vector.y - min_v);
    return res / (max_v - min_v);
}

void main()
{
    fragOut.uv = normalize_range_vec2(aPos.xz, 0, 1.0);
    fragOut.worldPos = aPos;
    fragOut.normal = vec3(0, 1.0, 0);
    outViewPos = uboProjectionView.viewPos;
    gl_Position = uboProjectionView.projection * uboProjectionView.view * vec4(aPos, 1.0);
}