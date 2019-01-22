#ifndef VKHR_SAMPLE_VOLUME_GLSL
#define VKHR_SAMPLE_VOLUME_GLSL

// Samples volume at 'volume_origin' with world dimensions 'volume_size' at the 'fragment_position'.
vec4 sample_volume(sampler3D volume, vec3 fragment_position, vec3 volume_origin, vec3 volume_size) {
    return texture(volume, (fragment_position - volume_origin) / volume_size);
}

#endif
