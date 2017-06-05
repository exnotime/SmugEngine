#pragma once
#include "../SubSystem.h"
class SSRender : public SubSystem {
  public:
	SSRender();
	~SSRender();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();
  private:

};

