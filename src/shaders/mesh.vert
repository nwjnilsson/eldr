#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "inputstructures.glsl"

// Outputs
layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec3 out_color;

struct Vertex {
    vec3 pos;
    float uv_x;
    vec3 normal;
    float uv_y;
    vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer {
    Vertex vertices[];
};

layout(push_constant) uniform Constants {
    mat4 world_matrix;
    VertexBuffer vertex_buffer;
} push_constants;

layout(set = 2, binding = 0) uniform ModelData {
    mat4 model_matrix;
} model_data;

void main() {
    Vertex v = push_constants.vertex_buffer.vertices[gl_VertexIndex];
    vec4 position = vec4(v.pos, 1.0f);
    gl_Position = scene_data.viewproj * push_constants.world_matrix *
            model_data.model_matrix * position;

    out_uv.x = v.uv_x;
    out_uv.y = v.uv_y;
    out_normal = (model_data.model_matrix * push_constants.world_matrix *
            vec4(v.normal, 0.f)).xyz;
    out_color = v.color.xyz * material_data.color_factors.xyz;
}
