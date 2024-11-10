#version 450

// Inputs
layout(location = 0) in vec3 in_pos;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_uv;

// Outputs
layout(location = 0) out vec3 out_color;
layout(location = 1) out vec2 out_uv;

// Uniform
layout(set = 0, binding = 0) uniform UniformBufferObject {
  mat4 model;
  mat4 view;
  mat4 proj;
} ubo;

void main() {
  gl_Position = ubo.proj * ubo.view * ubo.model * vec4(in_pos, 1.0);
  out_color = in_color;
  out_uv = in_uv;
}
