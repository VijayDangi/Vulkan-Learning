#version 450

layout(binding = 0) uniform UniformBufferObject
{
    vec2 foo;                   // Offset   0
    mat4 modelMatrix;           // Offset  16
    mat4 viewMatrix;            // Offset  64
    mat4 projectionMatrix;      // Offset 128
} ubo;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4( inPosition, 0.0, 1.0);
    fragColor = inColor;
}
