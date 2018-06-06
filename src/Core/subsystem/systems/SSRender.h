#pragma once
#include "../SubSystem.h"
#include "../../Timer.h"

namespace smug {
class SSRender : public SubSystem {
  public:
	SSRender();
	~SSRender();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();
  private:
	Timer m_Timer;
};
}

