#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec3 outColor; // output a color to the fragment shader
out vec2 TexCoord;
out vec3 outNormal;

uniform mat4 transform;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
    outColor = aColor;
    outNormal = aNormal;
    TexCoord = aTexCoord;
}