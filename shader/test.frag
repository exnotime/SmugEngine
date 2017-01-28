#version 430 core
layout(location = 0) in vec3 NormalW;

layout(location = 0) out vec4 outColor;
void main(){
    outColor = vec4(1,0,1,1) *(dot(normalize(NormalW), normalize(vec3(0.2f, -1.0f, 0.3f))) * 0.5 + 0.5);
    outColor.a = 1;
}
