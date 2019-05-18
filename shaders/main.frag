#version 330

in vec3 f_WorldPos;
in vec4 f_Color;
in vec3 f_Normal;
in vec3 f_ScreenPos;

uniform samplerCube u_CubeMap;
uniform sampler2D u_BackNormal;
uniform sampler2D u_BackDepth;
uniform vec3 u_CamPos;
uniform mat4 u_Projection;
uniform mat4 u_View;

layout(location = 0) out vec4 out_Color;
layout(location = 1) out vec4 out_Bloom;

float computeFresnel(float IOR, float LdotH)
{
	float g = sqrt(IOR * IOR - 1.0 + LdotH * LdotH);
	float a = g - LdotH;
	float b = g + LdotH;
	float c = LdotH * b - 1.0;
	float d = LdotH * a + 1.0;
	
	return 0.5 * a * a / (b * b) * (c * c / (d * d) + 1.0);
}

float depthAt(vec2 pos)
{
    return texture(u_BackDepth, (pos + 1.0) * 0.5).r * 2.0 - 1.0;
}

vec3 normalAt(vec2 pos)
{
    return normalize(texture(u_BackNormal, (pos + 1.0) * 0.5).rgb * 2.0 - 1.0);
}

vec3 raytraceRefraction(vec3 V, vec3 N)
{
    //Attention car le pas est hardcode ici mais il depends de la taille de la fenetre
    vec3 orig = normalize(refract(V, N, 1.0 / 1.45));
    vec3 dir  = normalize((u_Projection * u_View * vec4(orig, 0.0)).xyz) * 0.001953125;

    if(dir.z <= 0.0)
        return orig;

    vec3 pos  = f_ScreenPos;
    vec3 res  = vec3(0.0);

    for(int i = 0; i < 16; i++) {
        pos += dir;

        if(pos.z >= depthAt(pos.xy)) {
            res = normalAt(pos.xy);
            break;
        }
    }

    if(dot(res, res) != 0.0)
        orig = normalize(refract(orig, -res, 1.0 / 1.45));
    
    return orig;
}

void main()
{
    vec3 L  = normalize(vec3(0.0, -0.3333, 1.0));
    vec3 N  = normalize(f_Normal);
    vec3 V  = normalize(f_WorldPos - u_CamPos);
    vec3 R  = normalize(reflect(V, N));
    vec3 R2 = raytraceRefraction(V, N);
	vec3 H  = normalize(L + V);
    
    float NdotL = max(0.1, dot(N, -L));
	float fresnel = computeFresnel(1.45, dot(L, H));
	
	//vec3 diffuse = f_Color.rgb * NdotL;
	vec3 diffuse   = texture(u_CubeMap, R2).rgb * f_Color.rgb;
	vec3 specular  = texture(u_CubeMap, R).rgb;
    vec3 final     = mix(diffuse, specular, fresnel);
    float luma     = dot(final, vec3(0.2126, 0.7152, 0.0722));
    
    out_Color = vec4(final, 1.0);
    
    if(luma > 0.75)
        out_Bloom = vec4(final, 1.0);
    else
        out_Bloom = vec4(vec3(0.0), 1.0);
}
