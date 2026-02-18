#version 450

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec3 in_color;
layout(location = 2) in vec2 in_uv;
layout(location = 3) in vec3 in_view_vector;
layout(location = 4) in vec3 in_light_vector;

layout(binding = 1) uniform sampler2D u_sampler_color_map;

layout(location = 0) out vec4 o_frag_color;

// We use this constant to control the flow of the shader depending on the lighting model selected at pipeline creation time
layout(constant_id = 0) const int LIGHTING_MODEL = 0;
layout(constant_id = 1) const float PARAM_TOON_DESATURATION = 0.0f;

void main()
{
    switch(LIGHTING_MODEL)
    {
        case 0: // Phong
        {
            vec3 ambient = in_color * vec3(0.25);
            vec3 N = normalize(in_normal);
            vec3 L = normalize(in_light_vector);
            vec3 V = normalize(in_view_vector);
            vec3 R = reflect(-L, N);
            vec3 diffuse = max(dot(N, L), 0.0) * in_color;
            vec3 specular = pow(max(dot(R, V), 0.0), 32.0) * vec3(0.75);
            o_frag_color = vec4(ambient + diffuse * 1.75 + specular, 1.0);
        }
        break;

        case 1: // Toon
        {
            vec3 N = normalize(in_normal);
            vec3 L = normalize(in_light_vector);
            float intensity = dot(N, L);
            vec3 color;
                 if(intensity > 0.98)   { color = in_color * 1.5; }
            else if(intensity > 0.9)    { color = in_color * 1.0; }
            else if(intensity > 0.5)    { color = in_color * 0.6; }
            else if(intensity > 0.25)   { color = in_color * 0.4; }
            else                        { color = in_color * 0.2; }

            // desaturate
            color = vec3( mix( color, vec3( dot( vec3( 0.2126, 0.7152, 0.0722), color)), PARAM_TOON_DESATURATION));
            o_frag_color = vec4(color, 1.0);
        }
        break;

        case 2: // Textured
        {
            vec3 N = normalize(in_normal);
            vec3 L = normalize(in_light_vector);
            vec3 V = normalize(in_view_vector);
            vec3 R = reflect(-L, N);

            vec4 color = texture(u_sampler_color_map, in_uv).rrra;
            vec3 ambient = color.rgb * vec3(0.25) * in_color;
            vec3 diffuse = max(dot(N, L), 0.0) * color.rgb;
            float specular = pow(max(dot(R, V), 0.0), 32.0) * color.a;

            o_frag_color = vec4( ambient + diffuse + vec3(specular), 1.0);
        }
        break;
    }
}
