#version 330

in vec2 f_TexCoord;

uniform sampler2D u_Texture;
uniform float u_Exposure;
uniform float u_BloomThreshold;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Bloom;

void main()
{
    vec3 final = texture(u_Texture, f_TexCoord).rgb * pow(2.0, u_Exposure);
    float luma = dot(final, vec3(0.2126, 0.7152, 0.0722));
    
    out_Color = vec4(final, 1.0);
    
    if(luma > u_BloomThreshold)
        out_Bloom = vec4(final, 1.0);
    else
        out_Bloom = vec4(vec3(0.0), 1.0);
}
