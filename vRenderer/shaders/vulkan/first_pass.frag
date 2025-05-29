#version 450

#define MAX_LIGHT_SOURCES 10

layout(location = 0) in vec3 fragCol;
layout(location = 1) in vec2 fragUv;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) flat in uint fragTextured;
layout(location = 4) flat in vec3 outViewPos;
layout(location = 5) in vec3 outFragPos;

layout(set = 1, binding = 0) uniform sampler2D diffuseMap;
layout(set = 2, binding = 0) uniform sampler2D specularMap;

struct Light {

    vec4 color;            // Offset: 0
    vec4 position;         // Offset: 16
    vec4 direction;        // Offset: 32
    
    // 1 - directional
    // 2 - point
    // 3 - spot
    int type;             // Offset: 60
    float ambientStrength; // Offset: 48
    float specularStrength;// Offset: 52
    int shininess;         // Offset: 56
    
    float constant;        // Offset: 64
    float linear;          // Offset: 68
    float quadratic;       // Offset: 72
    float cutOff;          // Offset: 76
    
    float outerCutOff;     // Offset: 80
};

layout(set = 3, binding = 0) uniform LightsUniform {
    Light lights[MAX_LIGHT_SOURCES];
} lightUniform;

layout(location = 0) out vec4 outColor;     // final output color

vec3 getPhongComponent(Light light, vec3 lightDir, vec3 normal, vec3 viewDir);
float getAttenuationFactor(Light light, float lightDistance);

vec3 applyDirectionalLight(Light ligth, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection);
vec3 applyPointLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection);
vec3 applySpotLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection);

void main()
{
    // Fragment's normal in view space
    vec3 norm = normalize(fragNormal);
    // Viewing direction
    vec3 viewDir = normalize(outViewPos - outFragPos);

    vec3 shading = vec3(0);
    vec3 branch = vec3(0);
    for(int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if(lightUniform.lights[i].type == 1)
        {
            branch.x = 1.0;
            shading += applyDirectionalLight(lightUniform.lights[i], norm, viewDir, lightUniform.lights[i].position.xyz, lightUniform.lights[i].direction.xyz);
        }
        else if(lightUniform.lights[i].type == 2)
        {
            branch.y = 1.0;
            shading += applyPointLight(lightUniform.lights[i], norm, viewDir, lightUniform.lights[i].position.xyz, lightUniform.lights[i].direction.xyz);
        }
        else if(lightUniform.lights[i].type == 3)
        {
            branch.z = 1.0;
            shading += applySpotLight(lightUniform.lights[i], norm, viewDir, lightUniform.lights[i].position.xyz, lightUniform.lights[i].direction.xyz);
        }
    }

    // Unshaded fragment color calculation
    vec3 result = vec3(1.0);
    if(fragTextured != 0)
    {      
        result = texture(diffuseMap, fragUv).xyz;
    }
    else
    {
        result = vec3(1.0, 1.0, 1.0);
    }

    // Light application
    result = shading * result;

    outColor = vec4(result, 1.0);
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
vec3 applyDirectionalLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection)
{
    vec3 lightDir = normalize(-lightDirection);
    return getPhongComponent(light, lightDir, normal, viewDir);
}

// Returns a shading impact of a Point light source (light.type == 1)
vec3 applyPointLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection)
{
    vec3 lightDir = lightPos - outFragPos; 
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);

    vec3 phong = getPhongComponent(light, lightDir, normal, viewDir);
    phong *= getAttenuationFactor(light, lightDistance);
    return phong;
}

// Returns a shading impact of a Spot (Flashlight) light source (light.type == 2)
vec3 applySpotLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection)
{
    vec3 lightDir = lightPos - outFragPos; 
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);

    // Spotlight calculation
    float theta = dot(lightDir, normalize(-lightDirection));
    
    if(theta > light.outerCutOff) 
    {       
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        float attenuation = getAttenuationFactor(light, lightDistance);
        return attenuation * intensity * getPhongComponent(light, lightDir, normal, viewDir);
    }

    return vec3(0);
}