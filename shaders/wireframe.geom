#version 330

layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

uniform mat4 u_Projection;
uniform mat4 u_View;

in vec3 g_WorldPos[];
in vec3 g_Normal[];
out float f_IsNormal;

void main()
{
	gl_Position = gl_in[0].gl_Position; f_IsNormal = 0.0; EmitVertex();
	gl_Position = gl_in[1].gl_Position; f_IsNormal = 0.0; EmitVertex();
	gl_Position = gl_in[2].gl_Position; f_IsNormal = 0.0; EmitVertex();
	gl_Position = gl_in[0].gl_Position; f_IsNormal = 0.0; EmitVertex();
	EndPrimitive();
	
	vec3 center = (g_WorldPos[0] + g_WorldPos[1] + g_WorldPos[2]) / 3.0;
	vec3 normal = normalize((g_Normal[0] + g_Normal[1] + g_Normal[2]) / 3.0);
	
	vec4 p1 = u_Projection * u_View * vec4(center, 1.0);
	vec4 p2 = u_Projection * u_View * vec4(center + normal * 0.1, 1.0);
	
	gl_Position = p1; f_IsNormal = 1.0; EmitVertex();
	gl_Position = p2; f_IsNormal = 1.0; EmitVertex();
	EndPrimitive();
}
