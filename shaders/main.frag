#version 330

in vec4 f_Color;
in vec3 f_Normal;

out vec4 out_Color;

void main()
{
    vec3 sunDir = normalize(vec3(0.0, -0.3333, 1.0));
    vec3 nrm = normalize(f_Normal);
    float NdotL = max(0.1, dot(nrm, -sunDir));
    
    out_Color = vec4(f_Color.rgb * NdotL, f_Color.a);
}
