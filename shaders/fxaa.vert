#version 330
#define FXAA_SUBPIX_SHIFT (1.0/4.0)

layout(location = 0) in vec2 in_Vertex;

uniform vec2 u_InvTexSize;

out vec4 f_TexCoord;

void main()
{
    gl_Position = vec4(in_Vertex, 1.0, 1.0);
    f_TexCoord.xy = (in_Vertex + 1.0) / 2.0;
	f_TexCoord.zw = f_TexCoord.xy - (u_InvTexSize * (0.5 + FXAA_SUBPIX_SHIFT));
}
