#version 460 core
#extension GL_NV_ray_tracing : enable
layout(rgba8, binding = 0) writeonly restrict uniform image2D output_img;

layout(binding = 1) uniform PerFrame{
    mat4 invViewProj;
    vec4 CamPos;
    vec4 Lightdir;
};

layout(set = 0, binding = 2) uniform accelerationStructureNV g_WorldAS;
layout(location = 0) rayPayloadNV vec4 ColorPayload;

void main(){
    uvec2 screenPos = uvec2(gl_LaunchIDNV.xy);
    //spawn ray
    vec2 uv = screenPos / vec2(gl_LaunchSizeNV.xy);
    //calc clipspace pos
    vec2 hcs = ((vec2(screenPos) + vec2(0.5)) * 2) / gl_LaunchSizeNV.xy - vec2(1);
    //transform back into world space at depth 0.0
    vec4 posH =  (invViewProj * vec4(hcs.x, hcs.y, 0.0, 1.0));
    vec3 ro = posH.xyz / posH.w;
    vec3 rd = normalize(ro - CamPos.xyz);

    traceNV(g_WorldAS, 0, 0xff, 0, 0, 0, ro, 0.1f, rd, 1000.0f, 0);

    imageStore(output_img, ivec2(screenPos), ColorPayload);
}