#version 430 core

layout(location = 0) in vec3 posW;

layout (set = 0, binding = 0) uniform CBuffer{
	mat4 ViewProj;
}

void main(){
	gl_Position = ViewProj * vec4(posW,1);
}