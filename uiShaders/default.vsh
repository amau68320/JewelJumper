#version 330

layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec4 in_Color;
layout(location = 2) in vec2 in_TexCoord;

uniform mat4 u_Projection;

out vec4 frag_Color;
out vec2 frag_TexCoord;

void main()
{
    gl_Position = u_Projection * vec4(in_Pos, 1.0);
    frag_Color = in_Color / vec4(255.0);
    frag_TexCoord = in_TexCoord;
}
