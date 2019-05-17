#version 330

in vec2 f_TexCoord;

uniform sampler2D u_Texture;
uniform sampler2D u_BloomTex;
uniform vec2 u_InvTexSize;

layout(location = 0) out vec4 out_Color;

void main()
{
    //Main color
    const float gamma = 2.6;
    vec3 color = texture(u_Texture, f_TexCoord).rgb;
    
    //Bloom
    color += texture(u_BloomTex, f_TexCoord).rgb * 0.1964825501511404;
    color += texture(u_BloomTex, f_TexCoord + vec2(0.0,  1.411764705882353  * u_InvTexSize.y)).rgb * 0.2969069646728344;
    color += texture(u_BloomTex, f_TexCoord + vec2(0.0, -1.411764705882353  * u_InvTexSize.y)).rgb * 0.2969069646728344;
    color += texture(u_BloomTex, f_TexCoord + vec2(0.0,  3.2941176470588234 * u_InvTexSize.y)).rgb * 0.09447039785044732;
    color += texture(u_BloomTex, f_TexCoord + vec2(0.0, -3.2941176470588234 * u_InvTexSize.y)).rgb * 0.09447039785044732;
    color += texture(u_BloomTex, f_TexCoord + vec2(0.0,  5.176470588235294  * u_InvTexSize.y)).rgb * 0.010381362401148057;
    color += texture(u_BloomTex, f_TexCoord + vec2(0.0, -5.176470588235294  * u_InvTexSize.y)).rgb * 0.010381362401148057;
  
    //Reinhard tone mapping
    vec3 mapped = pow(color / (color + 1.0), vec3(1.0 / gamma));
    out_Color = vec4(mapped, 1.0);
}
