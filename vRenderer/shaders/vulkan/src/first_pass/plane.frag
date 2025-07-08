#version 450 core

#define MAX_LIGHT_SOURCES 10
#define GLOBAL_AMBIENT_STRENGTH 0.1

in FragInfo
{
    layout(location = 0) vec2 uv;
    layout(location = 1) vec3 normal;
    layout(location = 2) vec3 worldPos;
} fragIn; 

layout(location = 3) in vec3 outViewPos;

layout(location = 0) out vec4 FragColor;

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
    Light lights[10];
} uboLight;

vec4 selectBetween(sampler2D map, vec4 color);
vec3 getDiffuseSpecularImpact(Light light, vec3 lightDir, vec3 normal, vec3 viewDir);
float getAttenuationFactor(Light light, float lightDistance);

vec3 applyDirectionalLight(Light ligth, vec3 normal, vec3 viewDir);
vec3 applyPointLight(Light light, vec3 normal, vec3 viewDir);
vec3 applySpotLight(Light light, vec3 normal, vec3 viewDir);

void main()
{    
    // Fragment's normal in view space
    vec3 norm = vec3(0, 1.0, 0);
    // Viewing direction
    vec3 viewDir = normalize(outViewPos - fragIn.worldPos);

    // Default fragment color
    vec3 result = selectBetween(diffuseMap, uboMaterial.diffuseColor).xyz;

    vec3 shading = GLOBAL_AMBIENT_STRENGTH * selectBetween(ambientMap, uboMaterial.ambientColor).xyz;
    for(int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if(uboLight.lights[i].type == 1)
        {
            shading += applyDirectionalLight(uboLight.lights[i], norm, viewDir);
        }
        else if(uboLight.lights[i].type == 2)
        {
            shading += applyPointLight(uboLight.lights[i], norm, viewDir);
        }
        else if(uboLight.lights[i].type == 3)
        {
            shading += applySpotLight(uboLight.lights[i], norm, viewDir);
        }
    }

    FragColor = vec4(result * shading, 1.0);

    // Applying fragment's opacity. It comes either from texture map or set universally for material
    FragColor.a = max(texture(opacityMap, fragIn.uv).r, uboMaterial.opacity);
}

// Define what color to use between texture and its fallback color
// (if texture is present - color should be nulled)
vec4 selectBetween(sampler2D map, vec4 color)
{
    return mix(texture(map, fragIn.uv), vec4(color.xyz, 0.0), float(length(color.xyz) > 0.0));
}

// Returns a vector of Phong shading impact
vec3 getDiffuseSpecularImpact(Light light, vec3 lightDir, vec3 normal, vec3 viewDir)
{
    // Diffuse calculation
    vec3 diffuse = vec3(max(dot(normal, lightDir), 0.0));

    // Specular calculation
    //vec3 reflectDir = reflect(-lightDir, normal);  
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), max(uboMaterial.shininess, 1.0));
    vec3 specColor = selectBetween(specularMap, uboMaterial.specularColor).xyz;
    vec3 specular = specColor.xyz * spec;
    
    // TODO inspect this closely

    return  (diffuse + specular) * light.color.xyz;
}

// Returns a factor of light intensity decreased over distance
float getAttenuationFactor(Light light, float lightDistance)
{
    float attenuation = 1.0 / (
        light.constant + 
        light.linear *  lightDistance +
        light.quadratic * (lightDistance * lightDistance)
    );
    return attenuation;
}

// Returns a shading impact of a Directional light source (light.type == 0)
vec3 applyDirectionalLight(Light light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction.xyz);
    return getDiffuseSpecularImpact(light, lightDir, normal, viewDir);
}

// Returns a shading impact of a Point light source (light.type == 1)
vec3 applyPointLight(Light light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = light.position.xyz - fragIn.worldPos; 
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);

    vec3 phong = getDiffuseSpecularImpact(light, lightDir, normal, viewDir);
    phong *= getAttenuationFactor(light, lightDistance);
    return phong;
}

// Returns a shading impact of a Spot (Flashlight) light source (light.type == 2)
vec3 applySpotLight(Light light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = light.position.xyz - fragIn.worldPos; 
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);

    // Spotlight calculation
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    
    if(theta > light.outerCutOff) 
    {       
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        float attenuation = getAttenuationFactor(light, lightDistance);
        return attenuation * intensity * getDiffuseSpecularImpact(light, lightDir, normal, viewDir);
    }

    return vec3(0);
}