#pragma once
#include <stdint.h>
namespace smug {
	class SubSystem {
	public:
		SubSystem() {}
		~SubSystem() {}

		virtual void Startup() = 0;
		virtual void Shutdown() = 0;
		virtual void Update(const double deltaTime) = 0;

		void SetID(uint32_t id) {
			m_ID = id;
		}
		uint32_t m_ID = 0;
	};
}