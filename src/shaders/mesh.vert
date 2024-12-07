#version 450

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_buffer_reference : require

#include "input_structures.glsl"

// Outputs
layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec3 out_normal;
layout(location = 2) out vec4 out_color;

struct Vertex {
  vec3 pos;
  float uv_x;
  vec3 normal;
  float uv_y;
  vec4 color;
};

layout(buffer_reference, std430) readonly buffer VertexBuffer{
  Vertex vertices[];
};

layout( push_constant ) uniform constants {
  mat4 render_matrix;
  VertexBuffer vertex_buffer;
} push_constants;


layout(set = 1, binding = 0) uniform ModelData {
  mat4 mvp_mat;
} model_data;

void main() {
  Vertex v = push_constants.vertex_buffer.vertices[gl_VertexIndex];
  vec4 position = vec4(v.pos, 1.0f);
  gl_Position = scene_data.viewproj * push_constants.render_matrix * position;

  out_uv.x = v.uv_x;
  out_uv.y = v.uv_y;
  out_normal = (push_constants.render_matrix * vec4(v.normal, 0.f)).xyz
  out_color = v.color.xyz * material_data.color_factors.xyz;
}
