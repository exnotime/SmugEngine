
uint LoadPSO(){
    uint p = CreatePSO("prototype.shader");
    AddShader(p, "assets/shaders/default.vert", VERTEX);
    AddShader(p, "assets/shaders/default.frag", FRAGMENT);
    AddDefaultAttachmentInfo(p);
    SetPrimitiveTopology(p, PrimitiveTopology::TriangleList);
    SetDepthStencilStateInfo(p, false, false, 0x0);
    SetRasterStateInfo(p, CullMode::Back, FrontFace::CounterClockwise, false, false, PolygonMode::Fill, false, 0,0,0,0);
    return p;
}