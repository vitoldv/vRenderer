#version 450

#define MAX_LIGHT_SOURCES 10
#define GLOBAL_AMBIENT_STRENGTH 0.1

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

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

struct Light {

    // Only xyz components matter in all three vectors
    vec4 color;            
    vec4 position;        
    vec4 direction;      
    
    // 1 - directional
    // 2 - point
    // 3 - spot
    int type;                   
    
    float constant;        
    float linear;         
    float quadratic;     

    float cutOff;          
    float outerCutOff;   
};

layout(set = 3, binding = 0) uniform UboLight {
    Light lights[MAX_LIGHT_SOURCES];
} uboLight;

layout(set = 4, binding = 0) uniform UboDynamicColor {
    vec4 color;
} uboColor;

layout(location = 0) out vec4 FragColor; 

void main()
{
    FragColor = vec4(0.04, 0.28, 0.26, 1.0);
}