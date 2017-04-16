#define sqr(x) x * x
#define PI 3.141592653589793
#define M_INV_PI 0.31830988618379067153776752674503
#define MOD3 vec3(443.8975,397.2973, 491.1871)
#define EPS 0.00001
#define GAMMA 1.0
#define saturate(x) clamp(x, 0.0, 1.0)
layout(binding = 5) uniform samplerCube g_DiffEnvMap;
layout(binding = 6) uniform samplerCube g_SpecEnvMap;
layout(binding = 7) uniform sampler2D g_IBLTex;


float LambertDiffuse(vec3 normal, vec3 lightDir){
    float ndotl = dot(normal, -lightDir);
    return max(0,ndotl);
}

float BlinnSpecular(vec3 normal, vec3 lightDir, vec3 toEye, float shiny){
    vec3 reflected = reflect(lightDir, normal);
    float vdotr = max(0, dot(reflected, toEye));
    return pow(vdotr, shiny);
}

float Visibillity(float roughness, float ndotv, float ndotl){
	float m2 = sqr(roughness);
	float visV = ndotl * sqrt(ndotv * (ndotv - ndotv * m2) + m2);
	float visL = ndotv * sqrt(ndotl * (ndotl - ndotl * m2) + m2);
	return 0.5 / max(visV + visL, EPS);
}

float GeometricAtt(float hdotn, float vdotn, float vdoth, float ndotl)
{
	float a = (2 * hdotn * vdotn) / vdoth;
	float b = (2 * hdotn * ndotl) / vdoth;
	float c = min(a,b);
	return min(1,c);
}

float Distrobution(float roughness, float ndoth){
	float m2 = sqr(roughness);
	float d = (ndoth * m2 - ndoth) * ndoth + 1.0;
	return m2 / (d * d* PI);
}

float GGX_D(float roughness, float ndoth){
	float rSqr = sqr(roughness);
	float rSqr2 = sqr(rSqr);
	float t = sqr(ndoth) * (rSqr2 - 1) + 1;
	return rSqr2 / (PI * t * t);
}

vec3 Fresnel(vec3 specular, float vdoth){
	return saturate(50 * specular) * specular + (1.0 - specular) * pow((1.0 - vdoth), 5.0);
}

vec3 F_Schlick(vec3 f0, float vdoth){
	return f0 + (1.0 - f0) * pow(1.0f - vdoth, 5.0f);
}

float IORToF0(float n){
	return pow(n - 1, 2) / pow(n + 1, 2);
}

float G1V ( float dotNV, float k ) {
	return 1.0 / (dotNV*(1.0 - k) + k);
}

vec3 CookTorranceSpecular(vec3 normal, vec3 lightDir, vec3 toEye, float roughness, float metal, vec3 baseColor) {
	vec3 halfWayVector = normalize(toEye + lightDir);
	float hdotl = saturate(dot(halfWayVector, lightDir));
	float ndotl = saturate(dot(normal, lightDir));
	float ndoth = saturate(dot(halfWayVector, normal));
	float ndotv = saturate(dot(toEye, normal));
	float vdoth = saturate(dot(halfWayVector, toEye));

	float ior = mix(1.5, 3.0, metal);
	float f0 = IORToF0(ior);
	vec3 F0 = vec3(f0,f0,f0);
	F0 = mix(F0, F0 * baseColor, metal);

	float alpha = roughness * roughness;
	float alphaSqr = alpha * alpha;

	float denom = ndoth * ndoth * (alphaSqr - 1.0) + 1.0;
	float D = alphaSqr / (PI * denom * denom);

	vec3 F = F_Schlick(F0, hdotl);

	float k = alpha / 2.0;
	float vis = G1V(ndotl, k) * G1V(ndotv, k);

	return D * F * vis;
}

vec3 CalcDirLight(vec3 lightDir, vec3 albedo, vec3 normal, vec3 toEye, float roughness, float metallic) {
	vec3 ld = -normalize(lightDir);
	vec3 spec = vec3(0,0,0);
	vec3 diff = vec3(0,0,0);
	float ndotl = dot(normal, ld);
	diff = max(ndotl, 0) * albedo;
	if(ndotl > 0){
		spec = CookTorranceSpecular(normal, ld, toEye, roughness, metallic, albedo);
		spec = saturate(spec);
	}
	diff = mix(diff, vec3(0.0,0.0,0.0), metallic);
	spec = mix(spec, spec * albedo, metallic);
	return (diff + spec);
}

vec3 AproximateIBLSpecular(vec3 F0 , float roughness, vec3 normal, vec3 toeye){
 	float NoV = saturate(dot(normal, toeye));
 	NoV = max(EPS, NoV);
 	vec3 R = 2 * dot(normal, toeye) * normal - toeye;
 	ivec2 texDim = textureSize(g_SpecEnvMap, 0);
	float numMips = ceil(log2(float(max(texDim.x,texDim.y)))) - 1.0f;
	float mipLevel = numMips * roughness;

	vec3 color = pow(textureLod(g_SpecEnvMap, R, mipLevel).rgb, vec3(GAMMA));
	vec2 envBRDF = texture(g_IBLTex, vec2(roughness, NoV)).rg;

	return color * (envBRDF.x * F0 + envBRDF.y);
 }

vec3 CalcIBLLight( vec3 inNormal, vec3 toeye, vec3 baseColor, float roughness, float metal)
{
	vec4 lightColor = vec4(0,0,0,1);
	float ior = mix(1.4, 3.0, metal);//ior for plastic and iron, metallic values should actually be fetched from a texture and be in rgb space but this will do for now
	float f0 = IORToF0(ior);
	vec3 F0 = vec3(f0,f0,f0);
	F0 = mix(F0, F0 * baseColor, metal);

 	vec3 irradiance = pow(texture(g_DiffEnvMap, inNormal).rgb, vec3(GAMMA));
 	vec3 diffuse = baseColor * irradiance;
 	vec3 specular = AproximateIBLSpecular(F0, roughness, inNormal, toeye);
 	specular = saturate(specular);
	return vec3(specular + diffuse * (vec3(1) - F0) * (1 - metal));
}

vec3 hash32(vec2 p){
	vec3 p3 = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yxz+19.19);
    return fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

vec3 ditherRGB(vec3 c, vec2 seed){
	return c + hash32(seed) / 255.0;
}
