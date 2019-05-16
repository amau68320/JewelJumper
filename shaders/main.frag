#version 330

in vec3 f_WorldPos;
in vec4 f_Color;
in vec3 f_Normal;

uniform samplerCube u_CubeMap;
uniform vec3 u_CamPos;

out vec4 out_Color;

float computeFresnel(float IOR, float LdotH)
{
	float g = sqrt(IOR * IOR - 1.0 + LdotH * LdotH);
	float a = g - LdotH;
	float b = g + LdotH;
	float c = LdotH * b - 1.0;
	float d = LdotH * a + 1.0;
	
	return 0.5 * a * a / (b * b) * (c * c / (d * d) + 1.0);
}

void main()
{
    vec3 L = normalize(vec3(0.0, -0.3333, 1.0));
    vec3 N = normalize(f_Normal);
    vec3 V = normalize(f_WorldPos - u_CamPos);
    vec3 R = normalize(reflect(V, N));
	vec3 H = normalize(L + V);
    
    float NdotL = max(0.1, dot(N, -L));
	float fresnel = computeFresnel(1.45, dot(L, H));
	
	//vec3 diffuse = f_Color.rgb * NdotL;
	vec3 diffuse = texture(u_CubeMap, refract(V, N, 1.0 / 1.45)).rgb * f_Color.rgb;
	vec3 specular = texture(u_CubeMap, R).rgb;
    
    out_Color = vec4(diffuse * (1.0 - fresnel) + specular * fresnel, 1.0);
}
