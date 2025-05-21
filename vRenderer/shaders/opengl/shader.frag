#version 460 core

precision highp float;

in vec3 outColor;
in vec2 outTexCoord;
in vec3 outNormal;
in vec3 outFragPos;
in vec3 outLightPos;

out vec4 FragColor;

uniform sampler2D outTexture;
uniform bool useTexture;

uniform vec3 ambientColor;
uniform vec3 lightColor;
uniform float ambientStrength;
uniform float specularStrength;
uniform int shininess;

void main()
{
    vec3 norm = normalize(outNormal);
    vec3 lightDir = normalize(outLightPos - outFragPos);  
 
    // Ambient calculation
    vec3 ambient = ambientStrength * lightColor;
    // Diffuse calculation
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    // Specular calculation
    vec3 viewDir = normalize(-outFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;  

    // Unshaded fragment color calculation
    vec3 result = vec3(1.0f);
    if(useTexture)
    {
        result = texture(outTexture, outTexCoord).xyz;
    }
    else
    {
        result = ambientColor;
    }

    // Light application
    result = (ambient + diffuse + specular) * result;

    FragColor = vec4(result, 1.0f);
}