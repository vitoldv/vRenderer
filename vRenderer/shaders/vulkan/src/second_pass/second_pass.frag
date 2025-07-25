#version 450

layout(input_attachment_index = 0, binding = 0) uniform subpassInput inputColor;        // Color ouput from subpass 1
layout(input_attachment_index = 1, binding = 1) uniform subpassInput inputDepth;        // Depth ouput from subpass 1

layout(set = 1, binding = 0) uniform UboPostProcessingFeatures {
    float userGammaFactor;
    float userBrightnessOffset;
} postProcessingFeatures;

layout(location = 0) out vec4 FragColor;

void main()
{
    // We can only grab the input data at the current fragment position
    // for example, if current fragment is [2, 4], we can only take pixel (fragment) at [2, 4] position from any input image
    // subpassLoad([input_image]).rgba - is a way to grab this color
    //color = subpassLoad(inputColor).rgba;   
    
    vec4 result = subpassLoad(inputColor).rgba;

    float gamma = postProcessingFeatures.userGammaFactor;
    float brightness = postProcessingFeatures.userBrightnessOffset;

    // brightress aims to adjust the color as user prefers
    result.rgb += brightness;
    // gamma aims to correct the result to look the same among different user screen setup
    // (this is the value to adjust once per game start-up ("make the logo dissapear"))
    result.rgb = pow(result.rgb, vec3(gamma));

    FragColor = result;
}
