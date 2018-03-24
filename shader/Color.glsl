#version 430 core

#ifdef VERTEX
layout (location = 0) in vec3 posL;
layout (location = 1) in vec3 NormalL;
layout (location = 2) in vec3 TangentL;
layout (location = 3) in vec2 TexCoord;

layout (location = 0) out vec3 PosW;
layout (location = 1) out vec3 NormalW;
layout (location = 2) out vec3 TangentW;
layout (location = 3) out vec2 TexCoordOut;
layout (location = 4) out vec3 BiNormOut;
layout (location = 5) out vec3 ColorOut;

layout (set = 0, binding = 0) uniform g_PerFrame {
    mat4 ViewProj;
    vec4 CamPos;
    vec4 LightDir;
    vec4 Material;
};

struct PerObject{
    mat4 World;
	vec4 Color;
};

#define MAX_OBJECTS 1024
layout (set = 2, binding = 0) buffer ObjectBuffer{
	PerObject g_PerObjects[MAX_OBJECTS];
};

layout (push_constant) uniform pc{
    uint index;
} pushConstants;

void main(){
    mat4 world = g_PerObjects[pushConstants.index + gl_InstanceIndex].World;
    gl_Position = ViewProj * ( world * vec4(posL,1));
    PosW = (world * vec4(posL,1)).xyz;
    NormalW = (world * vec4(NormalL,0)).xyz;
    TangentW = (world * vec4(TangentL,0)).xyz;
    BiNormOut = cross(NormalW, TangentW);
    TexCoordOut = TexCoord;
    ColorOut = g_PerObjects[pushConstants.index + gl_InstanceIndex].Color.rgb;
}

#endif

#ifdef FRAGMENT
layout(location = 0) in vec3 PosW;
layout(location = 1) in vec3 NormalW;
layout(location = 2) in vec3 TangentW;
layout(location = 3) in vec2 TexCoordOut;
layout(location = 4) in vec3 BiNormOut;
layout (location = 5) in vec3 ColorOut;

layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform WVP{
    mat4 view_proj;
    vec4 CamPos;
    vec4 LightDir;
    vec4 Material;
};
#include "lighting.glsl"

layout(set = 3, binding = 0) uniform sampler2D g_Material[4];

vec3 CalcBumpedNormal(vec3 Bump, vec3 Normal, vec3 Tangent, vec3 BiNorm){
    vec3 normal = normalize(Normal);
    vec3 tangent = normalize(Tangent);
    vec3 binorm = normalize(BiNorm);

    mat3 TBN = mat3(tangent,binorm,normal);
    vec3 newNormal = TBN * Bump;
    return normalize(newNormal);
}

void main(){
    vec3 bump = texture(g_Material[1], TexCoordOut).xyz * 2.0 - 1.0;
    vec3 normal = CalcBumpedNormal(bump, NormalW, TangentW, BiNormOut);
    vec3 lightDir = normalize(LightDir.xyz);
    vec3 toCam = normalize(CamPos.xyz - PosW);
    vec3 texColor = pow(texture(g_Material[0], TexCoordOut).rgb, vec3(GAMMA)) * ColorOut;
    vec3 mat = pow(texture(g_Material[2], TexCoordOut).rgb, vec3(GAMMA));

    mat.r *= mat.r;
    mat.r = saturate(mat.r * Material.r);
    mat.r = clamp( mat.r, 0.001, 0.999);
    mat.g = Material.g;

    vec3 lightColor = CalcDirLight(-lightDir, texColor, normal, toCam, mat.r, mat.g) * mat.b;
    vec3 ibl = CalcIBLLight( normal, toCam, texColor, mat.r, mat.g) * mat.b;
    outColor = saturate(vec4(lightColor + ibl, 1));
}
#endif
