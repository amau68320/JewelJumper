#version 330

in vec3 f_Normal;

layout(location = 0) out vec4 out_Color;

void main()
{
    vec3 N = normalize(f_Normal);
    out_Color = vec4((N + 1.0) * 0.5, 1.0);
}
