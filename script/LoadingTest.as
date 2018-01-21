typedef uint64 ResourceHandle;

ResourceHandle koopaModel;
uint koopaEntity;

void Load(){
	print("Loading from script is possible!!\n");
	ResourceHandle koopaModel = LoadModel("assets/models/KoopaTroopa/Koopa.obj");
	LoadModel("assets/models/sphere/sphere.obj");

	koopaEntity = CreateEntity();
	CreateTransformComponent(koopaEntity, vec3(0,100,0));
	CreateModelComponent(koopaEntity, koopaModel);
}