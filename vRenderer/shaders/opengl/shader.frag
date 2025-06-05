#version 460 core

precision highp float;

#define MAX_LIGHT_SOURCES 10
#define GLOBAL_AMBIENT_STRENGTH 0.1

in vec3 fragColor;
in vec2 fragUv;
in vec3 fragNormal;
in vec3 fragPos;

out vec4 FragColor;

struct Material
{
    sampler2D ambientMap;
    sampler2D diffuseMap;
    sampler2D specularMap;
    sampler2D opacityMap;

    vec3 ambientColor;
    vec3 diffuseColor;
    vec3 specularColor;
    float opacity;

    float shininess;
};

struct Light
{
    // 0 - none
    // 1 - directional
    // 2 - point
    // 3 - spot

    int type;
    vec3 color;

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

vec4 selectBetween(sampler2D map, vec3 color);
vec3 getDiffuseSpecularImpact(Light light, vec3 lightDir, vec3 normal, vec3 viewDir);
float getAttenuationFactor(Light light, float lightDistance);

vec3 applyDirectionalLight(Light ligth, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection);
vec3 applyPointLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection);
vec3 applySpotLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection);

void main()
{
    // Fragment's normal in view space
    vec3 norm = normalize(fragNormal);
    // Viewing direction
    vec3 viewDir = normalize(-fragPos);

    // Default fragment color
    vec3 result = selectBetween(material.diffuseMap, material.diffuseColor).xyz;
    
    // Ambient shading is global and do not come from all lighting sources
    vec3 shading = GLOBAL_AMBIENT_STRENGTH * selectBetween(material.ambientMap, material.ambientColor).xyz;
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

    FragColor = vec4(result * shading, 1.0);

    // Applying fragment's opacity. It comes either from texture map or set universally for material
    FragColor.a = max(texture(material.opacityMap, fragUv).r, material.opacity);
}

// Define what color to use between texture and its fallback color
// (if texture is present - color should be nulled)
vec4 selectBetween(sampler2D map, vec3 color)
{
    return mix(texture(map, fragUv), vec4(color, 0.0), float(length(color) > 0.0));
}

// Returns a vector of Phong diffuse and specular shading impact
vec3 getDiffuseSpecularImpact(Light light, vec3 lightDir, vec3 normal, vec3 viewDir)
{
    // Diffuse calculation
    vec3 diffuse = vec3(max(dot(normal, lightDir), 0.0));

    // Specular calculation
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(material.shininess, 2.0));
    vec3 specColor = selectBetween(material.specularMap, material.specularColor).xyz;
    vec3 specular = specColor.xyz * spec;
    
    // wait what the fuck is with types here
    // TODO inspect
    return (diffuse + specular) * light.color;
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
    return getDiffuseSpecularImpact(light, lightDir, normal, viewDir);
}

// Returns a shading impact of a Point light source (light.type == 1)
vec3 applyPointLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection)
{
    vec3 lightDir = lightPos - fragPos; 
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);

    vec3 phong = getDiffuseSpecularImpact(light, lightDir, normal, viewDir);
    phong *= getAttenuationFactor(light, lightDistance);
    return phong;
}

// Returns a shading impact of a Spot (Flashlight) light source (light.type == 2)
vec3 applySpotLight(Light light, vec3 normal, vec3 viewDir, vec3 lightPos, vec3 lightDirection)
{
    vec3 lightDir = lightPos - fragPos; 
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);

    // Spotlight calculation
    float theta = dot(lightDir, normalize(-lightDirection));
    
    if(theta > light.outerCutOff) 
    {       
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        float attenuation = getAttenuationFactor(light, lightDistance);
        return attenuation * intensity * getDiffuseSpecularImpact(light, lightDir, normal, viewDir);
    }

    return vec3(0);
}