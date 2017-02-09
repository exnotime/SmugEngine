#version 430 core
layout (location = 0) in vec3 posL;
layout (location = 1) in vec3 NormalL;
layout (location = 2) in vec2 TexCoord;

layout (location = 0) out vec3 PosW;
layout (location = 1) out vec3 NormalW;
layout (location = 2) out vec2 TexCoordOut;

layout (binding = 0) uniform WVP{
    mat4 wvp;
    vec4 CamPos;
    vec4 LightDir;
};

void main(){
    gl_Position = wvp * vec4(posL,1);
    PosW = posL;
    NormalW = NormalL;
    TexCoordOut = TexCoord;
}
