vec3 kajiya_kay_diffuse(vec3 color, vec3 tangent, vec3 light) {
    float lambert_diffuse = dot(tangent, light);
    float diffuse_squared = lambert_diffuse * lambert_diffuse;

    float diffuse = (1.0f - diffuse_squared);
    diffuse /=  sqrt(1.0f - diffuse_squared);

    return color * diffuse;
}
