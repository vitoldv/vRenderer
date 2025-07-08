// #version 450 core

#define MAX_LIGHT_SOURCES 10
#define GLOBAL_AMBIENT_STRENGTH 0.1

struct PhongMaterial
{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    float opacity;
    float shininess;
};

struct Light
{
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

// Returns a vector of Phong shading impact
vec3 phong_getDiffuseSpecularImpact(Light light, vec3 lightDir, vec3 normal, vec3 viewDir, PhongMaterial mat)
{
    // Diffuse calculation
    vec3 diffuse = vec3(max(dot(normal, lightDir), 0.0));

    // Specular calculation
    vec3 reflectDir = reflect(-lightDir, normal);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(mat.shininess, 2.0));
    vec3 specColor = mat.specular.xyz;
    vec3 specular = specColor.xyz * spec;
    
    // TODO inspect this closely

    return  (diffuse + specular) * light.color.xyz;
}

// Returns a shading impact of a Directional light source (light.type == 0)
vec3 phong_applyDirectionalLight(Light light, vec3 normal, vec3 viewDir, PhongMaterial mat)
{
    vec3 lightDir = normalize(-light.direction.xyz);
    return phong_getDiffuseSpecularImpact(light, lightDir, normal, viewDir, mat);
}

// Returns a shading impact of a Point light source (light.type == 1)
vec3 phong_applyPointLight(Light light, vec3 normal, vec3 viewDir, vec3 fragWorldPos, PhongMaterial mat)
{
    vec3 lightDir = light.position.xyz - fragWorldPos; 
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);

    vec3 phong = phong_getDiffuseSpecularImpact(light, lightDir, normal, viewDir, mat);
    phong *= getAttenuationFactor(light, lightDistance);
    return phong;
}

// Returns a shading impact of a Spot (Flashlight) light source (light.type == 2)
vec3 phong_applySpotLight(Light light, vec3 normal, vec3 viewDir, vec3 fragWorldPos, PhongMaterial mat)
{
    vec3 lightDir = light.position.xyz - fragWorldPos; 
    float lightDistance = length(lightDir);
    lightDir = normalize(lightDir);

    // Spotlight calculation
    float theta = dot(lightDir, normalize(-light.direction.xyz));
    
    if(theta > light.outerCutOff) 
    {       
        float epsilon = light.cutOff - light.outerCutOff;
        float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);
        float attenuation = getAttenuationFactor(light, lightDistance);
        return attenuation * intensity * phong_getDiffuseSpecularImpact(light, lightDir, normal, viewDir, mat);
    }

    return vec3(0);
}

vec4 phong_shade(vec3 viewWorldPos, vec3 fragNormal, vec3 fragWorldPos, PhongMaterial mat, Light[MAX_LIGHT_SOURCES] lights)
{
    // Fragment's normal in view space
    vec3 norm = normalize(fragNormal);
    // Viewing direction
    vec3 viewDir = normalize(viewWorldPos - fragWorldPos);

    // Default fragment color
    vec3 result = mat.diffuse.xyz;

    vec3 shading = GLOBAL_AMBIENT_STRENGTH * mat.ambient.xyz;
    for(int i = 0; i < MAX_LIGHT_SOURCES; i++)
    {
        if(lights[i].type == 1)
        {
            shading += phong_applyDirectionalLight(lights[i], norm, viewDir, mat);
        }
        else if(lights[i].type == 2)
        {
            shading += phong_applyPointLight(lights[i], norm, viewDir, fragWorldPos, mat);
        }
        else if(lights[i].type == 3)
        {
            shading += phong_applySpotLight(lights[i], norm, viewDir, fragWorldPos, mat);
        }
    }

    return vec4(result * shading, mat.opacity);
}