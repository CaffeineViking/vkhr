#ifndef VKHR_GPAA_GLSL
#define VKHR_GPAA_GLSL

// Based on Emil Persson's GPAA as shown in SIGGRAPH 2011 but
// with many simplifications. Since we're only rendering line
// geometry, the fragment shaders will only give us positions
// at the "center" of the lines (even if it has a thickness).
// For us to find the coverage of fragments with thick lines,
// we need the actual fragment positions. In GLSL one can use
// gl_FragCoord.xy that's in screen-space. Thus distance from
// the center of the line to the fragment we want to estimate
// the coverage of is simply: length(gl_FragCoord.xy - P) for
// an P in screen-space (giving you some distance in pixels).
//
// Usage is quite simple: render your lines with thickness T.
// screen_fragment: will most likely be gl_FragCoord.xy here,
// world_line: point "on" the line in world-space (position),
// view_projection: pre-multiplied based on your camera info,
// resolution: size of the rendering target (e.g.: 1280x720),
// line_thickness: "should" be T, but you can play with this.
// Returns coverage: [0.0, 1.0], where 0.0 means the fragment
// is outside the line with thickness T, and 1.0 means pixels
// covers the entire line, and therefore covers the fragment.
//
// To make things even less abstract here is how I call gpaa:
//
// 1.  float coverage = gpaa(gl_FragCoord.xy, fs_in.position,
//                           camera.projection * camera.view,
//                           vec2(1280.00f, 720.00f), 2.00f);
// 2.  coverage *= 0.3f; // Alpha that will be used to blend.
// 3.  vec4 frag_color = vec4(shading * occlusion, coverage);

float gpaa(vec2 screen_fragment,
           vec4 world_line,
           mat4 view_projection,
           vec2 resolution,
           float line_thickness) {
    // Transforms: world -> clip -> ndc -> screen.
    vec4 clip_line = view_projection * world_line;
    vec3 ndc_line = (clip_line.xyz / clip_line.w);
    vec2 screen_line = ndc_line.xy; // really NDC.

    // Transform NDC to screen-space to compare with samples.
    screen_line = (screen_line + 1.0f) * (resolution / 2.0f);

    // Distance is measured in screen-space grids.
    float d = length(screen_line-screen_fragment);

    // Finally the coverage is based on thickness.
    return 1.00f - (d / (line_thickness / 2.00f));
}

#endif
