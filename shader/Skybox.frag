#version 430 core
in vec3 Pos;
out vec4 Color;
layout(binding = 1) uniform samplerCube g_Tex;
void main(){
    Color = vec4(pow(texture(g_Tex, Pos).rgb, vec3(2.2)), 1);
}
