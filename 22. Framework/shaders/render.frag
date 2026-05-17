#version 450

layout (location = 0) in float in_normalizedValue;

layout (location = 0) out vec4 out_fragColor;

vec3 colorMap(float value)
{
    float t = clamp(value, 0.0, 1.0);

    vec3 slowColor = vec3(0.0, 0.2, 1.0);
    vec3 midColor = vec3(0.0, 1.0, 0.5);
    vec3 fastColor = vec3(1.0, 0.1, 0.1);

    if(t < 0.5)
    {
        return mix(slowColor, midColor, t * 2.0);
    }
    else
    {
        return mix(midColor, fastColor, (t - 0.5) * 2.0);
    }
}

void main()
{
    out_fragColor = vec4(colorMap(in_normalizedValue), 1.0);
}
