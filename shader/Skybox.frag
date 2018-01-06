#version 430 core
layout(location = 0) in vec3 Pos;
layout(location = 0) out vec4 Color;
layout(binding = 1) uniform samplerCube g_Tex;

#define MOD3 vec3(443.8975,397.2973, 491.1871)
vec3 hash32(vec2 p){
	vec3 p3 = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yxz+19.19);
    return fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

vec3 ditherRGB(vec3 c, vec2 seed){
	return c + hash32(seed) / 255.0;
}

void main(){
    Color = vec4( ditherRGB(pow(texture(g_Tex, Pos).rgb, vec3(2.2)),gl_FragCoord.xy), 1);
}
