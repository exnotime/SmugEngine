
uint LoadPSO(){
    uint p = CreatePSO("prototype.shader");
    AddShader(p, "assets/shaders/prototype.glsl", VERTEX);
    AddShader(p, "assets/shaders/prototype.glsl", FRAGMENT);
    AddDefaultAttachmentInfo(p);
    AddDefaultAttachmentInfo(p);
    AddDefaultAttachmentInfo(p);
    SetPrimitiveTopology(p, PrimitiveTopology::TriangleList);
    SetDepthStencilStateInfo(p, false, false, 0x0);
    SetRasterStateInfo(p, CullMode::Back, FrontFace::Clockwise, false, false, PolygonMode::Fill, false, 0,0,0,1);
    SetRenderPass(p, "ColorPass");
    return p;
}