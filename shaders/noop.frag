#version 330

in vec2 f_TexCoord;

uniform sampler2D u_Texture;

layout(location = 0) out vec4 o_Color;

void main()
{
    o_Color = texture(u_Texture, f_TexCoord);
}
