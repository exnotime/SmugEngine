#pragma once
#include "../SubSystem.h"
#include <Core/entity/Entity.h>
#include <glm/glm.hpp>
namespace smug {
	enum AXES {
		X,Y,Z,NUM_AXES
	};

	struct EntityCache;
	class SSWorldCreator : public SubSystem {
	public:
		SSWorldCreator();
		~SSWorldCreator();

		virtual void Startup();
		virtual void Update(const double deltaTime, Profiler* profiler);
		virtual void Shutdown();
	private:
		void PickFromCamera(const glm::vec2& pos);

		EntityCache * m_Cache;
		Entity m_ArrowEntities[NUM_AXES];
		Entity* m_SelectedEntity = nullptr;
	};
}

