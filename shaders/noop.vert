#version 330

layout(location = 0) in vec2 in_Vertex;

uniform mat3 u_Matrix;

out vec2 f_TexCoord;

void main()
{
    vec3 pos = u_Matrix * vec3(in_Vertex, 1.0);
    gl_Position = vec4(pos.xy, 1.0, 1.0);

    f_TexCoord = (in_Vertex + 1.0) * 0.5;
}
