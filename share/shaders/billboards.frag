#version 460 core

layout(location = 0) in PipelineData {
    vec2 coordinate;
} fs_in;

layout(binding = 1) uniform sampler2D image_sampler;

layout(location = 0) out vec4 color;

void main() {
    color = texture(image_sampler, fs_in.coordinate);
}
