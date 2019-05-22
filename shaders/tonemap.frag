#version 330

in vec2 f_TexCoord;

uniform sampler2D u_Texture;
uniform sampler2D u_BloomTex;

layout(location = 0) out vec4 out_Color;

void main()
{
    //Couleur princpale
    vec3 color = texture(u_Texture, f_TexCoord).rgb;
    
    //Bloom
    color += texture(u_BloomTex, f_TexCoord).rgb;
  
    //ACES Filmic Tone mapping par Krzysztof Narkowicz (luv u Shell32 <3)
    //Issu de https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
    vec3 mapped = (color * (2.51 * color + 0.04)) / (color * (2.51 * color + 0.59) + 0.14);
    out_Color = vec4(mapped, 1.0);
}
