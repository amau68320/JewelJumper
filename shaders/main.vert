#version 330

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec3 in_Color;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

out vec3 f_Color;

void main()
{
    gl_Position = u_Projection * u_View * u_Model * vec4(in_Vertex, 1.0);
    f_Color = in_Color;
}
