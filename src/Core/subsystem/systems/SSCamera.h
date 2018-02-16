#pragma once
#include "../SubSystem.h"
namespace smug {
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
}
