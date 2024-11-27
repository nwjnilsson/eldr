#version 450

// Inputs
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_color;

// Outputs
layout(location = 0) out vec2 out_uv;
layout(location = 1) out vec4 out_color;

// Uniform
layout(set = 0, binding = 0) uniform UniformBufferObject {
  mat4 mvp_mat;
} ubo;

void main() {
  out_uv = in_uv;
  out_color = vec4(in_color, 1.0);
  gl_Position = ubo.mvp_mat * vec4(in_pos, 1.0);
}
