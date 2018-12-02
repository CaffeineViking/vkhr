// Based on "Rendering Hair with Three Dimensional Textures" by J. T. Kajiya and T. L. Kay.
vec3 kajiya_kay(vec3 diffuse, vec3 specular, float p, vec3 tangent, vec3 light, vec3 eye) {
    float cosTL = dot(tangent, light);
    float cosTE = dot(tangent, eye);

    float cosTL_squared = cosTL*cosTL;
    float cosTE_squared = cosTE*cosTE;

    float one_minus_cosTL_squared = 1.0f - cosTL_squared;
    float one_minus_cosTE_squared = 1.0f - cosTE_squared;

    float sinTL = sqrt(one_minus_cosTL_squared);
    float sinTE = sqrt(one_minus_cosTE_squared);

    vec3 diffuse_colors  = diffuse  * sinTL;
    vec3 specular_colors = specular * clamp(pow(cosTL*cosTE + sinTL*sinTE, p), 0.0f, 1.0f);

    return diffuse_colors + specular_colors;
}
