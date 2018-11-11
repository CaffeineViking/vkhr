#version 460 core

layout(location = 0) in PipelineData {
    vec2 texture_coordinate;
} fs_in;

layout(location = 0) out vec4 color;

void main() {
    color = vec4(fs_in.texture_coordinate,
                 0.0f, 1.0f); // To debug.
}
