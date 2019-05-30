#version 430
#extension GL_GOOGLE_include_directive : require
layout(local_size_x = 8, local_size_y = 8) in;
layout(rgba8, binding = 0) writeonly restrict uniform image2D output_img;

layout(binding = 1) uniform sampler2D albedo;
layout(binding = 2) uniform sampler2D depthStencil;
layout(binding = 3) uniform sampler2D normals;
layout(binding = 4) uniform sampler2D material;
layout(binding = 5) uniform sampler2D shadows;

layout(binding = 6) uniform PerFrame {
    vec4 LightDir;
    mat4 InvViewProj;
    vec4 CamPos;
    vec2 ScreenSize;
};

#include "lighting.glsl"

float blur9(vec2 uv, vec2 resolution, vec2 direction) {
  float color = 0.0;
  const vec2 off1 = (vec2(1.3846153846) * direction) / resolution;
  const vec2 off2 = (vec2(3.2307692308) * direction) / resolution;
  color += texture(shadows, uv).r * 0.2270270270;
  color += texture(shadows, uv + off1).r * 0.3162162162;
  color += texture(shadows, uv - off1).r * 0.3162162162;
  color += texture(shadows, uv + off2).r * 0.0702702703;
  color += texture(shadows, uv - off2).r * 0.0702702703;
  return color;
}

float denoise(vec2 uv, vec2 resolution){
    float center = texture(shadows, uv).r;
    float color = 0.0;
    float total = 0.0;
    const vec2 inv_res = vec2(1.0) / resolution;
    for (float x = -1.0; x <= 1.0; x += 1.0) {
        for (float y = -1.0; y <= 1.0; y += 1.0) {
            vec2 offset = vec2(x,y) * inv_res;
            float s = texture(shadows, uv + offset).r;
            float weight = 1.0 - abs((s - center) * 0.25 + 0.25);
            weight = pow(weight, 4);
            color += s * weight;
            total += weight;
        }
    }
    return color / total;
}

void main(){
    ivec2 screenPos = ivec2(gl_GlobalInvocationID.xy);
   if(any(greaterThan(screenPos, ScreenSize)))
        return;
    vec2 uv = (screenPos+ vec2(0.5)) / vec2(ScreenSize);
    vec3 albedo = texture(albedo, uv).rgb;
    float depth = texture(depthStencil, uv).r;
    if(depth >= 0.9999999f){
        imageStore(output_img, ivec2(screenPos), vec4(albedo, 1));
        return;
    }
    vec3 normal = texture(normals, uv).xyz * 2.0 - 1.0;
    //calc world pos
    vec2 hcs = uv * 2.0f - 1.0f;
    vec4 posH =  (InvViewProj * vec4(hcs.x, hcs.y, depth, 1.0));
    vec3 posW = posH.xyz / posH.w;
    
    vec2 roughness_metal = texture(material, uv).rg;
    vec3 toEye = normalize(CamPos.xyz - posW);
    float shadow = denoise(uv, ScreenSize);//texture(shadows, uv).r;
    
    vec3 Color = CalcDirLight(-LightDir.xyz, albedo, normal, toEye, roughness_metal.x, roughness_metal.y) * 5 * shadow;
    Color += CalcIBLLight(normal, toEye, albedo, roughness_metal.x, roughness_metal.y);
    
    imageStore(output_img, ivec2(screenPos), vec4(Color, 1));
}