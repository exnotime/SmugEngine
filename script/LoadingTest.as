typedef uint64 ResourceHandle;

ResourceHandle koopaModel;
ResourceHandle planeModel;
uint koopaEntity;
uint planeEntity;

void Load(){
	print("Loading from script is possible!!\n");
	koopaModel = LoadModel("assets/models/KoopaTroopa/Koopa.obj");
	LoadModel("assets/models/sphere/sphere.obj");

	koopaEntity = CreateEntity();
	CreateTransformComponent(koopaEntity, vec3(5,-5,0), vec3(20));
	CreateModelComponent(koopaEntity, koopaModel);

	planeEntity = CreateEntity();
	planeModel = LoadModel("assets/models/plane/plane.obj");
	CreateTransformComponent(planeEntity, vec3(0,-4.5,0));
	CreateModelComponent(planeEntity, planeModel);
}