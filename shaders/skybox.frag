#version 330

in vec2 f_TexCoord;

uniform sampler2D u_Texture;

out vec4 out_Color;

void main()
{
    out_Color = texture(u_Texture, f_TexCoord);
}
