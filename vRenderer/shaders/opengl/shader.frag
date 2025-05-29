#version 460 core

precision highp float;

#define MAX_LIGHT_SOURCES 10

in vec3 outColor;
in vec2 outTexCoord;
in vec3 outNormal;
in vec3 outFragPos;

out vec4 FragColor;

uniform bool useMaterial;
uniform bool useDiffuseColor;

struct Material
{
    sampler2D diffuseMap;
    sampler2D specularMap;
    vec3 diffuseColor;
};

struct Light
{
    // 0 - none
    // 1 - directional
    // 2 - point
    // 3 - spot

    int type;

    vec3 color;

    // Phong
    float ambientStrength;
    float specularStrength;
    int shininess;

    // Attenuation
    float constant;
	float linear;
	float quadratic;

    // Directional and Spotlight component
    vec3 direction;

    // Spotlight components
    float cutOff;
    float outerCutOff;
};
// Position of a light calculated at vertex stage in view space (PointLight and SpotLight component)
in vec3 outLightPos[10];
// Direction of a light calculated at vertex stage in view space (DirectionalLight and SpotLight component)
in vec3 outLightDir[10];

uniform Light lightSources[MAX_LIGHT_SOURCES];
uniform Material material;

vec3 getPhongComponent(Light light, vec3 lightDir, vec3 normal, vec3 viewDir);
float getAttenuationFactor(Light light, float lightDistance);

vec3 applyDirectionalLight(Light ligth, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection);
vec3 applyPointLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection);
vec3 applySpotLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection);

void main()
{
    // Fragment's normal in view space
    vec3 norm = normalize(outNormal);
    // Viewing direction
    vec3 viewDir = normalize(-outFragPos);

    vec3 shading;
    for(int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if(lightSources[i].type == 1)
        {
            shading += applyDirectionalLight(lightSources[i], norm, viewDir, outLightPos[i], outLightDir[i]);
        }
        else if(lightSources[i].type == 2)
        {
            shading += applyPointLight(lightSources[i], norm, viewDir, outLightPos[i], outLightDir[i]);
        }
        else if(lightSources[i].type == 3)
        {
            shading += applySpotLight(lightSources[i], norm, viewDir, outLightPos[i], outLightDir[i]);
        }
    }

    // Unshaded fragment color calculation
    vec3 result = vec3(1.0);
    if(useMaterial && !useDiffuseColor)
    {
        result = texture(material.diffuseMap, outTexCoord).xyz;
    }
    else
    {
        result = material.diffuseColor;
    }

    // Light application
    result = shading * result;

    FragColor = vec4(result, 1.0);
}

// Returns a vector of Phong shading impact
vec3 getPhongComponent(Light light, vec3 lightDir, vec3 normal, vec3 viewDir)
{
    // Ambient calculation
    vec3 ambient = light.ambientStrength * light.color;
    // Diffuse calculation
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color;
    // Specular calculation
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), light.shininess);
    vec3 specular = texture(material.specularMap, outTexCoord).xyz * light.specularStrength * spec * light.color;
    
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