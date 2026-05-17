#version 450

struct Particle
{
    vec4 position;
    vec4 velocity;
    float age;
    float maxAge;
    vec2 padding;
};

layout (std430, set = 0, binding = 0) readonly buffer InputParticleBuffer
{
    Particle particles[];
};

layout (set = 0, binding = 1) uniform UniformBufferObject
{
    mat4 viewMatrix;
    mat4 projMatrix;
} ubo;

layout (location = 0) out float o_normalizedValue;

void main()
{
    uint index = gl_VertexIndex;

    Particle particle = particles[index];

    // Pass normalized velocity magnitude to fragment shader for coloring
    o_normalizedValue = length(particle.velocity.xyz);   // Assuming max velocity is 1.5 for normalization
    o_normalizedValue = clamp(o_normalizedValue / 200, 0.0, 1.0);

    gl_Position = ubo.projMatrix * ubo.viewMatrix * vec4(particle.position.xyz, 1.0);
    gl_PointSize = 1.0;
}