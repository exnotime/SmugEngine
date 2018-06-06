#pragma once
#include <Physics/PhysicsEngine.h>
namespace smug {
struct RigidBodyComponent {
	PhysicsBody* Body;
	static unsigned int Flag;
};
}

