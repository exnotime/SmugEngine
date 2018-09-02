struct DeviceSignature{
    vec2 ScreenSize;
    string Vendor;
    uint64 Memory
};

//For now only support 4 compute queues
enum DEVICE_QUEUE{
    COPY,
    GRAPHICS,
    COMPUTE0,
    COMPUTE1,
    COMPUTE2,
    COMPUTE3
};

enum RENDER_TARGET_FORMATS{
    RGBA8_UNORM,
    RGBA16_FLOAT,
    RGBA32_FLOAT,
    R8_UNORM,
    R16_FLOAT,
    R32_FLOAT,
    D32_DEPTH,
    D24S8_DEPTH_STENCIL
};

void InitPsuedoRenderPipe(DeviceSignature deviceSignature){
    vec2 rendersize = deviceSignature.ScreenSize;
    //Allocate rendertargets and buffers
    AllocateRenderTarget("HDR", RGBA16_FLOAT, rendersize);
    AllocateRenderTarget("ShadowMap", D32_DEPTH, )
}
//RenderPrograms are programs supplied by the graphics engine that are defined in C++. Example is GeometryPass for shading or shadowpass to build a shadowmap
//Render
//RenderScripts are "programs" made from scripts that are hot-reloadable. The RenderScript can define 4 functions
//void Init();
//void Render();
//void Reload();
//void Shutdown();
//The script is then referenced here with RenderScript.
//Scripts can also be defined in C++ 
void PsuedoRenderPipe(){
    //build up list of programs to create a rendergraph
    RenderProgram("ShadowPass");
    RenderScript("MarchingCubes");
    RenderProgram("ColorPass");
    RenderProgram("ToneMap", );
}