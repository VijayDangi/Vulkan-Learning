#version 450

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConstant {
    vec4 position_offset;
    vec4 color;
} pushConsts;

void main()
{
    // outColor = pushConsts.color;
    outColor = pushConsts.color;
}
