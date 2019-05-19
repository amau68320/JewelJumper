#version 330

uniform sampler2D u_Atlas;

in vec4 frag_Color;
in vec2 frag_TexCoord;

out vec4 out_Color;

void main()
{
    out_Color = vec4(frag_Color.rgb, texture(u_Atlas, frag_TexCoord).r * frag_Color.a);
}
