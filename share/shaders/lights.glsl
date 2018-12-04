struct Light {
    vec3 vector;
    float type;
    vec3 intensity;
    float cutoff;
};

layout(binding = 1) uniform Lights {
    Light data[16];
    int light_size;
} lights;
