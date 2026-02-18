#version 450

layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;
layout(location = 3) in vec4 a_color;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 projection;
    mat4 view;
    mat4 model;
    vec4 light_position;
} ubo;

layout(location = 0) out vec3 o_normal;
layout(location = 1) out vec3 o_color;
layout(location = 2) out vec2 o_uv;
layout(location = 3) out vec3 o_view_vector;
layout(location = 4) out vec3 o_light_vector;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    o_color = a_color.rgb;
    o_uv = (a_position.xz + a_position.xy + a_position.yz) * 0.8;

    vec4 world_pos = ubo.model * vec4( a_position, 1.0);
    gl_Position = ubo.projection * ubo.view * world_pos;

    o_normal = mat3(ubo.model) * a_normal;
    vec3 light_position = mat3(ubo.model) * ubo.light_position.xyz;
    o_light_vector = light_position - world_pos.xyz;
    o_view_vector = - vec3(ubo.view * world_pos).xyz;
}
