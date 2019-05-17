#version 330

layout(location = 0) in vec2 in_Vertex;
out vec2 f_TexCoord;

void main()
{
    gl_Position = vec4(in_Vertex, 1.0, 1.0);
    f_TexCoord = (in_Vertex + 1.0) / 2.0;
}
