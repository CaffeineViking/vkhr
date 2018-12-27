#ifndef VKHR_VOLUME_GLSL
#define VKHR_VOLUME_GLSL

struct AABB {
    vec3 origin;
    float radius;
    vec3 size;
    float volume;
};

layout(binding = 3) uniform sampler3D density_texture;

vec4 sample_volume(sampler3D volume, vec3 position, vec3 resolution, vec3 origin, vec3 size) {
    vec3 voxel_size = size / resolution;
    vec3 voxel_position = (position - origin) / voxel_size;
    vec3 voxel = voxel_position / resolution;
    return texture(volume, voxel);
}

#endif
