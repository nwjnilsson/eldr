#version 450

// Uniforms
layout(binding = 1) uniform sampler2D tex_sampler;

// Inputs
layout(location = 0) in vec3 in_color;
layout(location = 1) in vec2 in_uv;

// Outputs
layout(location = 0) out vec4 out_color;

void main() {
    out_color = texture(tex_sampler, in_uv);
}
