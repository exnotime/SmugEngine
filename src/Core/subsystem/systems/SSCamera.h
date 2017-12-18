#pragma once
#include "../SubSystem.h"
struct EntityCache;
class SSCamera : public SubSystem {
  public:
	SSCamera();
	~SSCamera();

	virtual void Startup();
	virtual void Update(const double deltaTime);
	virtual void Shutdown();
  private:
	  EntityCache* m_Cache;
};

