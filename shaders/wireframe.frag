#version 330

in float f_IsNormal;
out vec4 out_Color;

void main()
{
	out_Color = (1.0 - f_IsNormal) * vec4(1.0) + f_IsNormal * vec4(1.0, 0.0, 0.0, 1.0);
}
