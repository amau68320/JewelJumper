#version 330

in vec2 f_TexCoord;

uniform sampler2D u_Texture;
uniform sampler2D u_BloomTex;
uniform float u_Exposure;

layout(location = 0) out vec4 out_Color;

void main()
{
    //Couleur princpale
    vec3 color = texture(u_Texture, f_TexCoord).rgb;
    
    //Bloom
    color += texture(u_BloomTex, f_TexCoord).rgb;

    //Exposition
    color *= u_Exposure;
  
    //Tone mapping de Krzysztof Narkowicz (luv u Shell32 <3)
    vec3 mapped = (color * (2.51 * color + 0.04)) / (color * (2.51 * color + 0.59) + 0.14);
    out_Color = vec4(mapped, 1.0);
}
