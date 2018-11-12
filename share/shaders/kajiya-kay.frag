#version 460 core

layout(location = 0) in PipelineData {
    vec3 tangent;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    float transparency = length(fs_in.tangent) * 1.0;
    color = vec4(fs_in.tangent, transparency);
}
