#version 450 core
layout(location = 0) in vec3 fragUv;
layout (set = 1, binding = 0) uniform samplerCube skyboxSampler;
layout(location = 0) out vec4 FragColor;

void main()
{    
    FragColor = texture(skyboxSampler, fragUv);
    //FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}