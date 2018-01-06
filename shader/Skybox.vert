#version 430 core
layout(location = 0) in vec3 PosL;
layout(location = 0) out vec3 Pos;

layout(binding = 0) uniform PerFrameBuffer{
        mat4 g_ProjView;
        mat4 g_World;
};

void main(){
    vec4 posV = g_ProjView * (g_World * vec4(PosL,1));
    gl_Position = posV.xyww;
    Pos = PosL.xyz;
}
