#pragma once
#include "../SubSystem.h"
namespace smug {
	class SSScript : public SubSystem {
	public:
		SSScript();
		~SSScript();

		virtual void Startup();
		virtual void Update(const double deltaTime);
		virtual void Shutdown();
	private:
	};
}

