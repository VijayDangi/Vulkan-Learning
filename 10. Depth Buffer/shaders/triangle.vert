#version 450

layout(binding = 0) uniform UniformBufferObject
{
    mat4 modelMatrix;           // Offset  0
    mat4 viewMatrix;            // Offset 16
    mat4 projectionMatrix;      // Offset 64
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

void main()
{
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4( inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
