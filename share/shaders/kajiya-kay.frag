#version 450 core

layout(location = 0) in PipelineData {
    vec3 tangent;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    float transparency = length(fs_in.tangent) * 0.2;
    color = vec4(fs_in.tangent, transparency);
}
