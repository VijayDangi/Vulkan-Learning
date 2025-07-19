#version 450

layout(location = 0) in vec2 inPosition;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 modelMatrix;           // Offset  16
    mat4 viewMatrix;            // Offset  64
    mat4 projectionMatrix;      // Offset 128
} ubo;


layout(push_constant) uniform PushConstant {
    vec4 position_offset;
    vec4 color;
} pushConsts;

void main()
{
    vec4 world_position = ubo.modelMatrix * vec4( inPosition, 0.0, 1.0);
    world_position.xy = world_position.xy + pushConsts.position_offset.xy;
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * world_position;
}
