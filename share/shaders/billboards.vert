#version 460 core

vec2 positions[] = {
    { -1.0f, -1.0f },
    { +1.0f, +1.0f },
    { +1.0f, -1.0f },
    { +1.0f, +1.0f },
    { -1.0f, -1.0f },
    { -1.0f, +1.0f }
};

vec2 texture_coordinates[] = {
    { 0.0f, 0.0f },
    { 1.0f, 1.0f },
    { 1.0f, 0.0f },
    { 1.0f, 1.0f },
    { 0.0f, 0.0f },
    { 0.0f, 1.0f },
};

layout(binding = 0) uniform Transform {
    mat4 model;
    mat4 view;
    mat4 projection;
} transform;

layout(location = 0) out PipelineData {
    vec2 texture_coordinate;
} vs_out;

void main() {
    mat4 projection_view = transform.projection * transform.view;

    vec2 texture_coordinate = texture_coordinates[gl_VertexIndex];
    vec3 position = vec3(positions[gl_VertexIndex], 0.0f);

    vec4 world_position = transform.model * vec4(position, 1.0f);

    vs_out.texture_coordinate = texture_coordinate;
    gl_Position = projection_view * world_position;
}
