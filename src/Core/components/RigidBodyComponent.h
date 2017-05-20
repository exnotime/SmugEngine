#pragma once
#include <Physics/PhysicsEngine.h>

struct RigidBodyComponent {
	PhysicsBody* Body;
	static unsigned int Flag;
};

