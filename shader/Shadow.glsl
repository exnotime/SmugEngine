#version 430
#ifdef VERTEX 

layout (location = 0) in vec3 posL;

struct PerObject{
    mat4 World;
	vec4 Color;
};

#define MAX_OBJECTS 1024
layout (set = 1, binding = 0) buffer ObjectBuffer{
	PerObject g_PerObjects[MAX_OBJECTS];
};

layout (push_constant) uniform pc{
    uint index;
} pushConstants;

void main(){
	mat4 world = g_PerObjects[pushConstants.index + gl_InstanceIndex].World;
	//transform vertex to world space then in the geometry shader project to multiple viewports
    gl_Position = (world * vec4(posL,1));
}
#endif//end vertex

#ifdef GEOMETRY

#extension GL_ARB_viewport_array : enable

#define CASCADE_COUNT 4
layout (triangles, invocations = CASCADE_COUNT) in;
layout (triangle_strip, max_vertices = 3) out;

layout(set = 0, binding = 0) uniform LightViewBuffer{
	mat4 lightViewProj[CASCADE_COUNT];
};

void main(){
	for(int i = 0; i < gl_in.length(); ++i){
		vec4 posW = gl_in[i].gl_Position;
		gl_Position = lightViewProj[gl_InvocationID] * vec4(posW.xyz,1);
		gl_ViewportIndex = gl_InvocationID;
		gl_PrimitiveID = gl_PrimitiveIDIn;
		EmitVertex();
	}
	EndPrimitive();
}

#endif //end geometry