#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec3 outColor; // output a color to the fragment shader
out vec2 outTexCoord;
out vec3 outNormal;
out vec3 outFragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

void main()
{
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    outColor = aColor;
    outNormal = normalMatrix * aNormal;
    outTexCoord = aTexCoord;

    // world space position of a fragment
    outFragPos = vec3(model * vec4(aPos, 1.0f));
}