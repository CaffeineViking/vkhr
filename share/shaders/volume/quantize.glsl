#ifndef VKHR_QUANTIZE_GLSL
#define VKHR_QUANTIZE_GLSL

uint quantize_strand(vec3 tangent, uint density) {
    vec3 quantized_tangent = round(tangent * 7.0);
    return (         density          & 0x000000FF) << 24U |
           (uint(quantized_tangent.z) & 0x000000FF) << 16U |
           (uint(quantized_tangent.y) & 0x000000FF) <<  8U |
           (uint(quantized_tangent.x) & 0x000000FF);
}

vec4 dequantize_hair(uint quantized_hair_strand) {
    vec3 dequantized_tangent = vec3(float((quantized_hair_strand & 0x000000FF))
                                    float((quantized_hair_strand & 0x0000FF00) >>  8U)
                                    float((quantized_hair_strand & 0x00FF0000) >> 16U));
    return vec4(0.0f);
}

#endif
