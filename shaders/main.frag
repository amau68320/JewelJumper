#version 330

in vec3 f_WorldPos;
in vec4 f_Color;
in vec3 f_Normal;

uniform samplerCube u_CubeMap;
uniform vec3 u_CamPos;

out vec4 out_Color;

void main()
{
    vec3 sunDir = normalize(vec3(0.0, -0.3333, 1.0));
    vec3 nrm = normalize(f_Normal);
    vec3 viewVec = normalize(f_WorldPos - u_CamPos);
    vec3 refl = normalize(reflect(viewVec, nrm));
    
    float NdotL = max(0.1, dot(nrm, -sunDir));
    
    out_Color = vec4(texture(u_CubeMap, refl).rgb, 1.0);
}
