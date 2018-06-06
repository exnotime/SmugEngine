#pragma once
#include <Core/subsystem/SubSystem.h>
namespace smug {
class SSToken : public SubSystem {
  public:
	SSToken() {}
	~SSToken() {}

	virtual void Startup();
	virtual void Shutdown();
	virtual void Update(const double deltaTime);
};
}