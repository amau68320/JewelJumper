#version 330

layout(location = 0) in vec3 in_Pos;
layout(location = 2) in vec3 in_Normal;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

out vec3 g_WorldPos;
out vec3 g_Normal;

void main()
{
	vec4 wp = u_Model * vec4(in_Pos, 1.0);
    gl_Position = u_Projection * u_View * wp;
	
	g_WorldPos = wp.xyz;
	g_Normal = mat3(u_Model) * in_Normal;
}
