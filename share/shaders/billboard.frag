#version 460 core

layout(location = 0) in PipelineIn {
    vec2 texcoord;
} fs_in;

layout(binding  = 1) uniform sampler2D billboard_texture;

layout(location = 0) out vec4 color;

void main() {
    color = texture(billboard_texture, fs_in.texcoord);
}
