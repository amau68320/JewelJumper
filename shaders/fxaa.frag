#version 330
#define FxaaInt2 vec2
#define FxaaFloat2 vec2
#define FxaaTexLod0(t, p) textureLod(t, p, 0.0)
#define FxaaTexOff(t, p, o, r) textureLod(t, p + o * r, 0.0)
#define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_REDUCE_MUL   (1.0/8.0)
#define FXAA_SPAN_MAX     8.0
#define tex u_Texture
#define posPos f_TexCoord
#define rcpFrame u_InvTexSize

in vec4 f_TexCoord;

uniform sampler2D u_Texture;
uniform vec2 u_InvTexSize;

out vec4 o_Color;

void main()
{
	vec3 rgbNW = FxaaTexLod0(tex, posPos.zw).xyz;
	vec3 rgbNE = FxaaTexOff(tex, posPos.zw, FxaaInt2(1,0), rcpFrame).xyz;
	vec3 rgbSW = FxaaTexOff(tex, posPos.zw, FxaaInt2(0,1), rcpFrame).xyz;
	vec3 rgbSE = FxaaTexOff(tex, posPos.zw, FxaaInt2(1,1), rcpFrame).xyz;
	vec3 rgbM  = FxaaTexLod0(tex, posPos.xy).xyz;

	vec3 luma = vec3(0.299, 0.587, 0.114);
	float lumaNW = dot(rgbNW, luma);
	float lumaNE = dot(rgbNE, luma);
	float lumaSW = dot(rgbSW, luma);
	float lumaSE = dot(rgbSE, luma);
	float lumaM  = dot(rgbM,  luma);

	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	
	vec2 dir; 
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
	
	float dirReduce = max(
		(lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
		FXAA_REDUCE_MIN);
	float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);
	dir = min(FxaaFloat2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX), 
		  max(FxaaFloat2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX), 
		  dir * rcpDirMin)) * rcpFrame.xy;
	
	vec4 rgbA = (1.0/2.0) * (
		FxaaTexLod0(tex, posPos.xy + dir * (1.0/3.0 - 0.5)) +
		FxaaTexLod0(tex, posPos.xy + dir * (2.0/3.0 - 0.5)));
	vec4 rgbB = rgbA * (1.0/2.0) + (1.0/4.0) * (
		FxaaTexLod0(tex, posPos.xy + dir * (0.0/3.0 - 0.5)) +
		FxaaTexLod0(tex, posPos.xy + dir * (3.0/3.0 - 0.5)));
	float lumaB = dot(rgbB.xyz, luma);
	if((lumaB < lumaMin) || (lumaB > lumaMax)) o_Color = rgbA;
	else o_Color = rgbB;
}
