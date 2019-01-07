#ifndef VKHR_VOLUME_GLSL
#define VKHR_VOLUME_GLSL

vec4 sample_volume(sampler3D volume, vec3 position, vec3 origin, vec3 size) {
    return texture(volume, (position - origin) / size);
}

#endif
