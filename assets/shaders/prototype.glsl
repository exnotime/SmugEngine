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
layout (location = 6) out vec3 ViewOut;

layout (set = 0, binding = 0) uniform WVP{
    mat4 viewProj;
    mat4 view;
    vec4 CamPos;
    vec4 LightDir;
    vec4 Material;
    mat4 light_view_proj[4];
    vec4 NearFar;
    float ShadowSplits[4];
};

struct PerObject{
    mat3x4 World;
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
    mat3x4 world = g_PerObjects[pushConstants.index + gl_InstanceIndex].World;
    PosW = (vec4(posL,1) * world).xyz;
    gl_Position = viewProj * vec4(PosW,1);
    ViewOut = (view * vec4(PosW,1)).xyz;
    NormalW = (vec4(NormalL,0) * world).xyz;
    TangentW = (vec4(TangentL,0) * world).xyz;
    BiNormOut = cross(NormalW, TangentW);
    TexCoordOut = TexCoord;
    ColorOut = g_PerObjects[pushConstants.index + gl_InstanceIndex].Color.rgb;
}

#endif

#ifdef FRAGMENT
#extension GL_GOOGLE_include_directive : require

layout(location = 0) in vec3 PosW;
layout(location = 1) in vec3 NormalW;
layout(location = 2) in vec3 TangentW;
layout(location = 3) in vec2 TexCoordOut;
layout(location = 4) in vec3 BiNormOut;
layout(location = 5) in vec3 ColorOut;
layout(location = 6) in vec3 ViewOut;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outMaterial;

layout (set = 0, binding = 0) uniform WVP{
    mat4 view_proj;
    mat4 view;
    vec4 CamPos;
    vec4 LightDir;
    vec4 Material;
    mat4 light_view_proj[4];
    vec4 NearFar;
    float ShadowSplits[4];
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
    outNormal = vec4(NormalW * 0.5 + 0.5, 1);
    outAlbedo = vec4(ColorOut,1);
    vec3 mat = pow(texture(g_Material[2], TexCoordOut).rgb, vec3(GAMMA));
    outMaterial = vec4(mat.r, mat.g, mat.b, 1.0);
}
#endif
