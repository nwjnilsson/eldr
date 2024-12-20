#version 450

// Uniforms
layout (binding = 0) uniform sampler2D font_sampler;

// Inputs
layout (location = 0) in vec2 in_uv;
layout (location = 1) in vec4 in_color;

// Outputs
layout (location = 0) out vec4 out_color;

void main() {
	out_color = in_color * texture(font_sampler, in_uv);
}
