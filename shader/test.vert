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

layout (set = 0, binding = 0) uniform g_PerFrame {
    mat4 vp;
    vec4 CamPos;
    vec4 LightDir;
};

layout (set = 2, binding = 0) uniform g_PerObject{
	mat4 world;
	vec4 color;
};

void main(){
    gl_Position = vp * ( world * vec4(posL,1));
    PosW = (world * vec4(posL,1)).xyz;
    NormalW = (world * vec4(NormalL,0)).xyz;
    TangentW = (world * vec4(TangentL,0)).xyz;
    BiNormOut = cross(NormalW, TangentW);
    TexCoordOut = TexCoord;
}
