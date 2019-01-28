#ifndef VKHR_IMAGE_ATOMIC_RGBA8_AVG_GLSL
#define VKHR_IMAGE_ATOMIC_RGBA8_AVG_GLSL

vec4 convRGBA8ToVec4(uint val){
    return vec4(float((val&0x000000FF)),
                float((val&0x0000FF00)>>8U),
                float((val&0x00FF0000)>>16U),
                float((val&0xFF000000)>>24U));
}
uint convVec4ToRGBA8(vec4 val){
    return (uint(val.w)&0x000000FF)<<24U |
           (uint(val.z)&0x000000FF)<<16U |
           (uint(val.y)&0x000000FF)<<8U  |
           (uint(val.x)&0x000000FF);
}
void imageAtomicRGBA8Avg(layout(r32ui) coherent volatile uimage3D imgUI, ivec3 coords, vec4 val) {
    val.rgb *= 255.0f; // Optimise following calculations
    uint newVal = convVec4ToRGBA8(val);
    uint prevStoredVal = 0; uint curStoredVal;
    // Loop as long as destination value gets changed by other threads
    while ((curStoredVal = imageAtomicCompSwap(imgUI, coords, prevStoredVal, newVal)) != prevStoredVal) {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4(curStoredVal);
        rval.xyz = (rval.xyz*rval.w); // Denormalize
        vec4 curValF = rval+val; // Add new value
        curValF.xyz /= (curValF.w); // Renormalize
        newVal = convVec4ToRGBA8(curValF);
    }
}

#endif
