#version 460 core
#extension GL_NV_ray_tracing : enable

layout(location = 0) rayPayloadInNV vec4 ColorPayload;

void main(){
    ColorPayload = vec4(1.0f, 0, 0, 0.0f);
}