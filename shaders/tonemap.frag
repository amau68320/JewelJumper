#version 330

in vec2 f_TexCoord;

uniform sampler2D u_Texture;

out vec4 out_Color;

void main()
{
    const float gamma = 2.6;
    vec3 color = texture(u_Texture, f_TexCoord).rgb;
  
    //Reinhard tone mapping
    vec3 mapped = pow(color / (color + 1.0), vec3(1.0 / gamma));
    out_Color = vec4(mapped, 1.0);
}
