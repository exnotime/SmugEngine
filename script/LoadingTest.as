typedef uint64 ResourceHandle;

ResourceHandle koopaModel;
ResourceHandle planeModel;
ResourceHandle timmyModel;
uint koopaEntity;
uint planeEntity;
uint timmyEntity;

void Load(){
	//array<string> rts = {"HDR", "DepthStencil", "GBuffer0_AlbedoMatIndex", "GBuffer1_Normal_AO"};
	//float renderScale = 1.0f;
	//int width = 1920 * renderScale;
	//int height = 1080 * renderScale;
	//AllocateRenderTarget(width, height, RENDER_TARGET_FORMAT::R16G16B16A16, "HDR");
	//AllocateRenderTarget(width, height, RENDER_TARGET_FORMAT::D24S8, "DepthStencil");
	//AllocateRenderTarget(width, height, RENDER_TARGET_FORMAT::R8G8B8A8, "GBuffer0_AlbedoMatIndex");
	//AllocateRenderTarget(width, height, RENDER_TARGET_FORMAT::R16G16B16A16, "GBuffer1_Normal_AO");

	//ResourceHandle computeshader = LoadShader("assets/shaders/compute.shader");
	//ResourceHandle tonemapShader = LoadShader("assets/shaders/ToneMap.shader");
	////build render pipeline
	//BeginRenderPass("Main");
	//array<string> colorPass = {"HDR", "DepthStencil"};
	//BindRenderTargets(colorPass);
	//Render(RENDER_KEY_OPAQUE | RENDER_KEY_TRANSPARENT | RENDER_KEY_DYNAMIC | RENDER_KEY_STATIC, 0);
	//Dispatch(tonemapShader, width / 32, height / 32, 1, RENDER_QUEUE_GFX);
	//EndRenderPass();

	koopaModel = LoadModel("assets/models/KoopaTroopa/Koopa.obj");
	timmyModel = LoadModel("assets/models/TimmyTommy/lrn00.obj");
	LoadModel("assets/models/sphere/sphere.obj");

	koopaEntity = CreateEntity();
	CreateTransformComponent(koopaEntity, vec3(5,-5,0), vec3(20));
	CreateModelComponent(koopaEntity, koopaModel);

	timmyEntity = CreateEntity();
	CreateTransformComponent(timmyEntity, vec3(10,-5,0), vec3(20));
	CreateModelComponent(timmyEntity, timmyModel);

	planeEntity = CreateEntity();
	planeModel = LoadModel("assets/models/plane/plane.obj");
	CreateTransformComponent(planeEntity, vec3(0,-10.,0));
	CreateModelComponent(planeEntity, planeModel);
	CreateRigidBodyComponent(planeEntity, PHYSICS_SHAPE::PLANE, 0, true, true);
}