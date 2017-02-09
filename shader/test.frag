#version 430 core
layout(location = 0) in vec3 PosW;
layout(location = 1) in vec3 NormalW;
layout(location = 2) in vec2 TexCoordOut;
layout(location = 0) out vec4 outColor;

layout (binding = 0) uniform WVP{
    mat4 wvp;
    vec4 CamPos;
    vec4 LightDir;
};

layout(binding = 1) uniform sampler2D g_Tex;

void main(){
    float ndotl = dot(normalize(NormalW), normalize(LightDir.xyz));
    float spec = 0.0f;
    float diff = max(0,ndotl);
    if(diff > 0){
        vec3 toCam = normalize(CamPos.xyz - PosW);
        vec3 reflected = reflect(-LightDir.xyz, NormalW);
        float ldotr = max(0, dot(reflected, toCam));
        spec = pow(ldotr, 8);
    }
    vec4 texColor = texture(g_Tex, TexCoordOut);
    vec4 lightColor = vec4(diff) + vec4(spec) + vec4(0.01f);
    outColor = clamp(vec4((texColor * lightColor).rgb , 1), 0, 1);
}
