typedef uint64 ResourceHandle;

ResourceHandle koopaModel;
ResourceHandle planeModel;
ResourceHandle A18Model;
ResourceHandle SkylineModel;
uint koopaEntity;
uint planeEntity;
uint A18Entity;
uint SkylineEntity;

void Load(){
	
	float renderScale = 1.0f;
	int width = 1920 * renderScale;
	int height = 1080 * renderScale;
	AllocateRenderTarget(width, height, RENDER_TARGET_FORMAT::R8G8B8A8, "Albedo");
	AllocateRenderTarget(width, height, RENDER_TARGET_FORMAT::R16G16B16A16, "Normals");
	AllocateRenderTarget(width, height, RENDER_TARGET_FORMAT::R8G8B8A8, "Material");
	AllocateRenderTarget(width, height, RENDER_TARGET_FORMAT::R8G8B8A8, "Shadows");
	AllocateRenderTarget(width, height, RENDER_TARGET_FORMAT::R16G16B16A16, "HDR");
	AllocateRenderTarget(width, height, RENDER_TARGET_FORMAT::D24S8, "DepthStencil");
	array<string> rts = {"Albedo", "Normals", "Material", "DepthStencil"};
	CreateRenderPass("ColorPass", rts);

	koopaModel = LoadModel("assets/models/KoopaTroopa/Koopa.obj");
	A18Model = LoadModel("assets/models/ignored/2019 May - Android 18/model/a18.fbx");
	SkylineModel = LoadModel("assets/models/ignored/Skyline/model/skyline.dae");
	LoadModel("assets/models/sphere/sphere.obj");

	koopaEntity = CreateEntity();
	CreateTransformComponent(koopaEntity, vec3(15,-10,0), vec3(20));
	CreateModelComponent(koopaEntity, koopaModel, true);
	AddToStaticQueue(koopaEntity);

	A18Entity = CreateEntity();
	CreateTransformComponent(A18Entity, vec3(10,-10,0), vec3(0.01));
	CreateModelComponent(A18Entity, A18Model, true);
	AddToStaticQueue(A18Entity);

	SkylineEntity = CreateEntity();
	CreateTransformComponent(SkylineEntity, vec3(20,-10,0), vec3(3));
	CreateModelComponent(SkylineEntity, SkylineModel, true);
	AddToStaticQueue(SkylineEntity);

	planeEntity = CreateEntity();
	planeModel = LoadModel("assets/models/plane/plane.obj");
	CreateTransformComponent(planeEntity, vec3(0,-10.,0));
	CreateModelComponent(planeEntity, planeModel, true);
	CreateRigidBodyComponent(planeEntity, PHYSICS_SHAPE::PLANE, 0, true, true);
	AddToStaticQueue(planeEntity);
}