#define sqr(x) x * x
#define PI 3.141592653589793
#define M_INV_PI 0.31830988618379067153776752674503
#define MOD3 vec3(443.8975,397.2973, 491.1871)
#define EPS 0.0001
#define GAMMA 2.2
#define saturate(x) clamp(x, 0.0, 1.0)
layout(set = 1,binding = 0) uniform sampler2D g_IBLTex;
layout(set = 1,binding = 1) uniform samplerCube g_IBLCube[2];


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

float DistributionGGX(vec3 N, vec3 H, float a){
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float nom    = a2;
    float denom  = (NdotH2 * (a2 - 1.0) + 1.0);
    denom        = PI * denom * denom;
	
    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float k)
{
    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return nom / denom;
}
  
float GeometrySmith(vec3 N, vec3 V, vec3 L, float k)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, k);
    float ggx2 = GeometrySchlickGGX(NdotL, k);
	
    return ggx1 * ggx2;
}

vec3 CookTorranceSpecular(vec3 normal, vec3 lightDir, vec3 toEye, float roughness, float metal, vec3 baseColor) {
	vec3 halfWayVector = normalize(toEye + -lightDir);
	float ndotl = saturate(dot(lightDir, normal));
	float ndotv = saturate(dot(toEye, normal));
	float ndoth = saturate(dot(halfWayVector, normal));

	vec3 F0 = mix(vec3(0.02f), baseColor, metal);

	float D = DistributionGGX(normal, halfWayVector, roughness);

	float k = pow(roughness + 1.0, 2) / 8.0;
	float N = GeometrySmith(normal, toEye, -lightDir, k);

	vec3 F = F_Schlick(F0, ndotv);

	return vec3(N * D * F) / ((4 * ndotl * ndotv) + 0.001);
}

vec3 CalcDirLight(vec3 lightDir, vec3 albedo, vec3 normal, vec3 toEye, float roughness, float metallic) {
	vec3 ld = normalize(lightDir);

	float ndotl = max(dot(normal, ld), 0);
	float ndotv = max(dot(normal, toEye), 0.0);
	vec3  h = normalize(ld + toEye);
	vec3 F0 = mix(vec3(0.04), albedo, metallic);

	float D = DistributionGGX(normal, h, roughness);
	float G = GeometrySmith(normal, toEye, lightDir, roughness);
	vec3 F = F_Schlick(F0, max(dot(h, toEye), 0.0));

	vec3 Ks = F;
	vec3 Kd = vec3(1.0) - Ks;
	Kd *= 1.0 - metallic;

	vec3 nom = D * F * G;
	float denom = 4.0 * ndotl * (ndotv + 0.01);
	vec3 spec = nom / denom;
	return saturate((((Kd * albedo) / PI) + spec) * ndotl);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 CalcIBLLight( vec3 inNormal, vec3 toeye, vec3 baseColor, float roughness, float metal)
{
	vec3 F0 = mix(vec3(0.02f), baseColor, metal);
 	vec3 irradiance = texture(g_IBLCube[1], inNormal).rgb;

 	float NoV = clamp(dot(normalize(inNormal), normalize(toeye)), 0.1, 1.0);
 	vec3 R = normalize(reflect(-toeye, inNormal));

 	vec3 F = fresnelSchlickRoughness(NoV, F0, roughness);
 	vec3 Kd = 1.0 - F;
 	Kd *= 1.0 - metal;

 	ivec2 texDim = textureSize(g_IBLCube[0], 0);
	float numMips = ceil(log2(float(max(texDim.x,texDim.y)))) - 1.0f;
	float mipLevel = numMips * roughness;

	vec3 color = textureLod(g_IBLCube[0], R, mipLevel).rgb;
	vec2 envBRDF = texture(g_IBLTex, vec2(NoV, roughness)).rg;

 	vec3 diffuse = baseColor * irradiance;
 	vec3 specular = saturate(color * (F * envBRDF.x + envBRDF.y));

	return saturate(Kd * diffuse + specular);
}

vec3 hash32(vec2 p){
	vec3 p3 = fract(vec3(p.xyx) * MOD3);
    p3 += dot(p3, p3.yxz+19.19);
    return fract(vec3((p3.x + p3.y)*p3.z, (p3.x+p3.z)*p3.y, (p3.y+p3.z)*p3.x));
}

vec3 ditherRGB(vec3 c, vec2 seed){
	return c + hash32(seed) / 255.0;
}
