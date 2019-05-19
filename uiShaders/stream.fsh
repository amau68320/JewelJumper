#version 330

in vec4 frag_Color;
in vec2 frag_TexCoord;

out vec4 out_Color;

void main()
{
    out_Color = frag_Color;
}

