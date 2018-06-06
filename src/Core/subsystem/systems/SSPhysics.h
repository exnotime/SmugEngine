#pragma once
#include "../SubSystem.h"
namespace smug {
class SSPhysics : public SubSystem {
  public:
	SSPhysics();
	~SSPhysics();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();
  private:
};
}

