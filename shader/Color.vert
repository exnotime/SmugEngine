#version 430 core
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
} optionalInstanceName;

void main(){
    mat4 world = g_PerObjects[optionalInstanceName.index].World;
    gl_Position = ViewProj * ( world * vec4(posL,1));
    PosW = (world * vec4(posL,1)).xyz;
    NormalW = (world * vec4(NormalL,0)).xyz;
    TangentW = (world * vec4(TangentL,0)).xyz;
    BiNormOut = cross(NormalW, TangentW);
    TexCoordOut = TexCoord;
    ColorOut = g_PerObjects[optionalInstanceName.index].Color.rgb;
}
