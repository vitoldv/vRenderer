#version 460 core

precision highp float;

out vec4 FragColor;
  
in vec3 outColor;
in vec2 TexCoord;
in vec3 outNormal;

uniform sampler2D outTexture;

void main()
{
    FragColor = texture(outTexture, TexCoord);
    //FragColor = vec4(TexCoord, 0.0f, 1.0f);
}