#version 450
#extension GL_ARB_shading_language_include : require

#include "utils.glsl"
#include "phong_shading.glsl"

in FragInfo
{
    layout(location = 0) vec3 color;
    layout(location = 1) vec2 uv;
    layout(location = 2) vec3 normal;
    layout(location = 3) vec3 worldPos;
} fragIn; 

layout(location = 4) flat in vec3 outViewPos;

// Material (sets 1 & 2)
layout(set = 1, binding = 0) uniform sampler2D ambientMap;
layout(set = 1, binding = 1) uniform sampler2D diffuseMap;
layout(set = 1, binding = 2) uniform sampler2D specularMap;
layout(set = 1, binding = 3) uniform sampler2D opacityMap;
layout(set = 2, binding = 0) uniform UboMaterial {
    vec4 ambientColor;
    vec4 diffuseColor;
    vec4 specularColor;
    float opacity;
    float shininess;
} uboMaterial;

layout(set = 3, binding = 0) uniform UboLight {
    Light lights[MAX_LIGHT_SOURCES];
} uboLight;

layout(set = 4, binding = 0) uniform UboDynamicColor {
    vec4 color;
} uboColor;

layout(location = 0) out vec4 FragColor; 

PhongMaterial phongMat;

void main()
{
    // set a PhongMat struct
    phongMat.ambient = selectBetween(ambientMap, uboMaterial.ambientColor, fragIn.uv);
    phongMat.diffuse = selectBetween(diffuseMap, uboMaterial.diffuseColor, fragIn.uv);
    phongMat.specular = selectBetween(specularMap, uboMaterial.specularColor, fragIn.uv);
    phongMat.opacity = max(texture(opacityMap, fragIn.uv).r, uboMaterial.opacity);
    phongMat.shininess = uboMaterial.shininess;

    FragColor = phong_shade(outViewPos, fragIn.normal, fragIn.worldPos, phongMat, uboLight.lights);
}