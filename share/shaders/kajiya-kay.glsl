// Based on "Rendering Hair with Three Dimensional Textures" by J. T. Kajiya and T. L. Kay.
vec3 kajiya_kay(vec3 diffuse, vec3 specular, float p, vec3 tangent, vec3 light, vec3 eye) {
    float cosTL = abs(dot(light, tangent));
    float cosTE = abs(dot(eye,   tangent));

    float cosTL_squared = cosTL*cosTL;
    float cosTE_squared = cosTE*cosTE;

    float one_minus_cosTL_squared = 1.0f - cosTL_squared;
    float one_minus_cosTE_squared = 1.0f - cosTE_squared;

    float sinTL = one_minus_cosTL_squared / sqrt(one_minus_cosTL_squared);
    float sinTE = one_minus_cosTE_squared / sqrt(one_minus_cosTE_squared);

    vec3 diffuse_colors = diffuse  * sinTL;
    vec3 specular_color = specular * pow(cosTL * cosTE + sinTL * sinTE, p);

    return diffuse_colors + specular_color;
}
