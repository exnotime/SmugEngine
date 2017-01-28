#version 430 core
layout (location = 0) in vec3 posL;
layout (location = 1) in vec3 NormalL;

layout (location = 0) out vec3 NormalW;

layout (binding = 1) uniform WVP{
    mat4 wvp;
};

void main(){
    gl_Position = wvp * vec4(posL,1);
    NormalW = NormalL;
}
