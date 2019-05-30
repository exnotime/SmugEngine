
uint LoadPSO(){
    uint p = CreatePSO("Lighting.shader");
    AddShader(p, "assets/shaders/ComputeLighting.glsl", COMPUTE);
    return p;
}