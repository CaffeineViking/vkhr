#ifndef VKHR_TEX2DPROJ_GLSL
#define VKHR_TEX2DPROJ_GLSL

// Projects a 3-D position onto a 2-D plane for sampling a texture.
vec4 tex2Dproj(sampler2D image, vec4 position, vec2 displacement) {
    vec4 texel = vec4(1.0f);
    vec3 projected_position = position.xyz / position.w;
    texel = texture(image, projected_position.st + displacement);
    return texel;
}

#endif
