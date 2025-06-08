#version 450

#extension GL_GOOGLE_include_directive : require
#include "inputstructures.glsl"

layout(location = 0) in vec2 in_uv;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec3 in_color;

layout(location = 0) out vec4 out_frag_color;

void main()
{
    vec2 mr = texture(metallic_rough_tex, in_uv).xy;
    float metallic = mr.x;
    float roughness = mr.y;

    vec3 N = normalize(in_normal);
    vec3 V = normalize(scene_data.viewdir); // TODO: camera direction, implement camera
    vec3 L = normalize(scene_data.sunlight_direction.xyz);
    vec3 H = normalize(V + L);

    float light_value = max(dot(in_normal, scene_data.sunlight_direction.xyz), 0.1f);
    // float light_value = max(dot(in_normal * texture(metal_rough_tex, in_uv).xyz,
    //             scene_data.sunlight_direction.xyz), 0.1f);

    vec3 color = in_color * texture(color_tex, in_uv).xyz;
    vec3 ambient = color * scene_data.ambient_color.xyz;

    out_frag_color = vec4(color * light_value * scene_data.sunlight_color.w + ambient, 1.0f);
}
