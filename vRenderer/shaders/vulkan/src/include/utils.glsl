// #version 450 core

// Define what color to use between texture and its fallback color
// (if texture is present - color should be nulled)
vec4 selectBetween(sampler2D map, vec4 color, vec2 fragUv)
{
    return mix(texture(map, fragUv), vec4(color.xyz, 0.0), float(length(color.xyz) > 0.0));
}