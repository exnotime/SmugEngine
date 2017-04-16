#version 430 core
layout(location = 0) in vec3 PosW;
layout(location = 1) in vec3 NormalW;
layout(location = 2) in vec2 TexCoordOut;

layout(location = 0) out vec4 outColor;

layout (set = 0, binding = 0) uniform WVP{
    mat4 wvp;
    mat4 world;
    vec4 CamPos;
    vec4 LightDir;
};
#include "lighting.glsl"

layout(set = 1, binding = 0) uniform sampler2D g_Albedo;
layout(set = 1, binding = 1) uniform sampler2D g_Normal;
layout(set = 1, binding = 2) uniform sampler2D g_Roughness;
layout(set = 1, binding = 3) uniform sampler2D g_Metal;

void main(){
    vec3 normal = normalize(NormalW);
    vec3 lightDir = normalize(LightDir.xyz);
    float diff = LambertDiffuse(normal, lightDir);
    vec3 toCam = normalize(CamPos.xyz - PosW);
    float spec = 0.0f;
    if(diff > 0){
        spec = BlinnSpecular(normal, lightDir, toCam, 8.0);
    }
    vec4 texColor = texture(g_Albedo, TexCoordOut);
    float lightColor = diff + spec + 0.01f;
    outColor = clamp(vec4((texColor * lightColor).rgb , 1), 0, 1);
}
