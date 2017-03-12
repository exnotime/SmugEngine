#version 430 core
in vec3 Pos;
out vec4 Color;
layout(binding = 1) uniform samplerCube g_Tex;
void main(){
    Color = texture(g_Tex, Pos);
}
