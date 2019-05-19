#version 330

uniform sampler2D u_Texture0;

in vec4 frag_Color;
in vec2 frag_TexCoord;

out vec4 out_Color;

void main()
{
	out_Color = texture(u_Texture0, frag_TexCoord) * frag_Color;

	if(out_Color.a == 0.0)
		discard;
}
