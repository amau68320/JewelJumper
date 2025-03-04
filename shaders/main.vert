#version 330

layout(location = 0) in vec3 in_Vertex;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec3 in_Normal;

uniform mat4 u_Projection;
uniform mat4 u_View;
uniform mat4 u_Model;

out vec3 f_WorldPos;
out vec4 f_Color;
out vec3 f_Normal;
out vec3 f_ScreenPos;

void main()
{
    vec4 worldPos = u_Model * vec4(in_Vertex, 1.0);
    vec4 finalPos = u_Projection * u_View * worldPos;
    gl_Position = finalPos;
    
    f_WorldPos = worldPos.xyz;
    f_Color = in_Color;
    f_Normal = mat3(u_Model) * in_Normal;
    f_ScreenPos = finalPos.xyz / finalPos.w;
}
