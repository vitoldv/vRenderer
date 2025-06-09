#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aUv;

out vec3 fragColor;
out vec2 fragUv;
out vec3 fragNormal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

#define outline_scale_factor 0.01

void main()
{
    mat4 modelViewMatrix = view * model;
    vec4 pos_view_original = modelViewMatrix * vec4(aPos, 1.0);
    vec3 model_normal_unit = normalize(aNormal);
    vec3 world_normal_unit = normalize(mat3(normalMatrix) * model_normal_unit);
    vec3 view_normal_unit = normalize(mat3(view) * world_normal_unit);
    float distance_from_camera = abs(pos_view_original.z);
    float view_space_extrusion = distance_from_camera * outline_scale_factor;
    
    vec4 new_pos_view = pos_view_original + vec4(view_normal_unit * view_space_extrusion, 0.0);
    gl_Position = projection * new_pos_view;
}