
uint LoadPSO(){
    uint p = CreatePSO("Color.shader");
    AddShader(p, "assets/shaders/color.glsl", VERTEX);
    AddShader(p, "assets/shaders/color.glsl", FRAGMENT);
    AddDefaultAttachmentInfo(p);
    AddDefaultAttachmentInfo(p);
    AddDefaultAttachmentInfo(p);
    SetPrimitiveTopology(p, PrimitiveTopology::TriangleList);
    SetDepthStencilStateInfo(p, true, true, 0x0);
    SetRasterStateInfo(p, CullMode::Back, FrontFace::CounterClockwise, false, true, PolygonMode::Fill, false, 0,0,0,0);
    SetRenderPass(p, "ColorPass");
    return p;
}