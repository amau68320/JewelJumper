#version 330

in float f_IsNormal;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Bloom;

void main()
{
	vec4 color = mix(vec4(1.0), vec4(1.0, 0.0, 0.0, 1.0), f_IsNormal);

	out_Color = color;
	out_Bloom = color; //A la base c'etait un bug, mais j'ai trouve ca style...
}
