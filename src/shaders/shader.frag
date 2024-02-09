#version 450

// Uniform
layout(binding = 1) uniform sampler2D tex_sampler;

// Inputs
layout(location = 0) in vec3 frag_color;
layout(location = 1) in vec2 tex_coord;

// Outputs
layout(location = 0) out vec4 out_color;

void main() {
    out_color = texture(tex_sampler, tex_coord);
}
