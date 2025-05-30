#version 450

#define MAX_LIGHT_SOURCES 10

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragPos;

layout(location = 4) flat in uint outTextured;
layout(location = 5) flat in vec3 outViewPos;

// Material (set = 1)
layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 1, binding = 1) uniform sampler2D specularMap;

struct Light {

    // Only xyz components matter in all three vectors
    vec4 color;            
    vec4 position;        
    vec4 direction;      
    
    // 1 - directional
    // 2 - point
    // 3 - spot
    int type;            
    float ambientStrength;
    float specularStrength;
    int shininess;         
    
    float constant;        
    float linear;         
    float quadratic;     
    float cutOff;          
    
    float outerCutOff;   
};

layout(set = 2, binding = 0) uniform UboLight {
    Light lights[MAX_LIGHT_SOURCES];
} uboLight;

layout(set = 3, binding = 0) uniform UboDynamicColor {
    vec4 color;
} uboColor;

layout(location = 0) out vec4 FragColor; 

vec3 getPhongComponent(Light light, vec3 lightDir, vec3 normal, vec3 viewDir);
float getAttenuationFactor(Light light, float lightDistance);

vec3 applyDirectionalLight(Light ligth, vec3 normal, vec3 viewDir);
vec3 applyPointLight(Light light, vec3 normal, vec3 viewDir);
vec3 applySpotLight(Light light, vec3 normal, vec3 viewDir);

void main()
{
    // Fragment's normal in view space
    vec3 norm = normalize(fragNormal);
    // Viewing direction
    vec3 viewDir = normalize(outViewPos - fragPos);

    vec3 shading = vec3(0);
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

    // Unshaded fragment color calculation
    vec3 result = vec3(1.0);
    if(outTextured != 0)
    {      
        result = texture(diffuseMap, fragUv).xyz;
    }
    else
    {
        result = vec3(1.0, 1.0, 1.0);
    }

    // Light application
    result = shading * result;

    FragColor = vec4(result, 1.0);
}

// Returns a vector of Phong shading impact
vec3 getPhongComponent(Light light, vec3 lightDir, vec3 normal, vec3 viewDir)
{
    // Ambient calculation
    vec3 ambient = light.ambientStrength * light.color.xyz;
    // Diffuse calculation
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color.xyz;
    // Specular calculation
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), light.shininess);
    vec3 specular = texture(specularMap, fragUv).xyz * light.specularStrength * spec * light.color.xyz;
    
    return ambient + diffuse + specular;
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
    return getPhongComponent(light, lightDir, normal, viewDir);
}

// Returns a shading impact of a Point light source (light.type == 1)
vec3 applyPointLight(Light light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = light.position.xyz - fragPos; 
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);

    vec3 phong = getPhongComponent(light, lightDir, normal, viewDir);
    phong *= getAttenuationFactor(light, lightDistance);
    return phong;
}

// Returns a shading impact of a Spot (Flashlight) light source (light.type == 2)
vec3 applySpotLight(Light light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = light.position.xyz - fragPos; 
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);

    // Spotlight calculation
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    
    if(theta > light.outerCutOff) 
    {       
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        float attenuation = getAttenuationFactor(light, lightDistance);
        return attenuation * intensity * getPhongComponent(light, lightDir, normal, viewDir);
    }

    return vec3(0);
}