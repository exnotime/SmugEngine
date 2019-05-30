#version 460 core
#extension GL_NV_ray_tracing : enable

layout(location = 0) rayPayloadInNV vec4 ColorPayload;

layout(binding = 1) uniform PerFrame{
    mat4 invViewProj;
    vec4 CamPos;
    vec4 Lightdir;
};

vec3 TriLerp3(vec3 a, vec3 b, vec3 c, float u, float v){
    return (1.0 - u - v) * a + b * u + c * v;
}

hitAttributeNV vec2 barycentricCoords;

void main(){
    ColorPayload = vec4(1);
    //test! for now just ignore non opaque objects
    //TODO: Get textures into this shader and test against alpha
    //reportIntersectionNV(0, );
    return;
    //ColorPayload = vec4(0);
    // vec3 worldPos = gl_WorldRayOriginNV + gl_WorldRayDirectionNV * gl_HitTNV;
    // uint indexOffset = g_InstanceIndexOffsets[gl_InstanceID] + gl_PrimitiveID * 3;
    // vec3 normal = TriLerp3( g_VertexNormals[g_IndexBuffer[indexOffset]],
    //                         g_VertexNormals[g_IndexBuffer[indexOffset + 1]],
    //                         g_VertexNormals[g_IndexBuffer[indexOffset + 2]],
    //                         barycentricCoords.x, barycentricCoords.y);
    // ColorPayload *= dot(normalize(normal), normalize(-Lightdir.xyz)) * vec4(0,1,0,1);
}