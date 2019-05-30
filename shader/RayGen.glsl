#version 460 core
#extension GL_NV_ray_tracing : enable
layout(rgba8, binding = 0) writeonly restrict uniform image2D output_img;

layout(binding = 1) uniform PerFrame{
    mat4 invViewProj;
    vec4 CamPos;
    vec4 Lightdir;
};
layout(binding = 3) uniform sampler2D depthStencil;
layout(binding = 4) uniform sampler2D normals;

layout(set = 0, binding = 2) uniform accelerationStructureNV g_WorldAS;
layout(location = 0) rayPayloadNV vec4 ColorPayload;



// Normal points outward for rays exiting the surface, else is flipped.
vec3 OffsetRay(const vec3 p, const vec3 n)
{
    const float origin = 1.0f / 32.0f;
    const float float_scale = 1.0f / 65536.0f; 
    const float int_scale = 256.0f;

    ivec3 of_i = ivec3(int_scale * n.x, int_scale * n.y, int_scale * n.z);

    vec3 p_i = vec3(
        float(int(p.x)+((p.x < 0) ? -of_i.x : of_i.x)),
        float(int(p.y)+((p.y < 0) ? -of_i.y : of_i.y)),
        float(int(p.z)+((p.z < 0) ? -of_i.z : of_i.z))
    );

    return vec3(  abs(p.x) < origin ? p.x+ float_scale*n.x : p_i.x,
                    abs(p.y) < origin ? p.y+ float_scale*n.y : p_i.y,
                    abs(p.z) < origin ? p.z+ float_scale*n.z : p_i.z);
}

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453) * 2.0 - 1.0;
}

void main(){
    uvec2 screenPos = uvec2(gl_LaunchIDNV.xy);
    //spawn ray
    vec2 uv = (screenPos+ vec2(0.5)) / vec2(gl_LaunchSizeNV.xy);
    float depth = texture(depthStencil, uv).r;
    if(depth >= 0.99999999){
        return;
    }
    vec3 normal = texture(normals, uv).xyz * 2.0 - 1.0;
    //calc clipspace pos
    vec2 hcs = uv * 2.0f - 1.0f;
    vec4 posH =  (invViewProj * vec4(hcs.x, hcs.y, depth, 1.0));
    vec3 ro = posH.xyz / posH.w;
    ro = ro + normal * 0.005;
    vec3 rd = normalize(-Lightdir.xyz);
    ColorPayload = vec4(1);
    const uint sampleCount = 8;
    const float delta = (1.0 / sampleCount);
    float shadow = 0.0f;
    for(uint i = 0; i < sampleCount; ++i){
        vec3 rayDir = rd + vec3(rand(uv + i),rand(uv + i * 16),rand(uv + i * 30)) * 0.05f; 
        traceNV(g_WorldAS, gl_RayFlagsTerminateOnFirstHitNV , 0xff, 0, 0, 0, ro, 0.001, rayDir, 1000.0, 0);
        shadow += ColorPayload.r * dot(rayDir, rd) * delta;
    }
    imageStore(output_img, ivec2(screenPos), vec4(shadow, 0, 0, 0));
}