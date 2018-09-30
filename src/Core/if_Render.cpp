#include "if_Render.h"
#include <assert.h>
#include "script/ScriptEngine.h"
#include <Core/GlobalSystems.h>
#include <Utility/Hash.h>
using namespace AngelScript;
namespace smug {
	namespace if_render {

#define DEF_ENUM(v) #v, v

		enum RENDER_TARGET_FORMAT {
			R8G8B8A8,
			R16G16B16A16,
			D32,
			D24S8,
			NUM_FORMATS
		};

		vk::Format RTFormatToVKFormat(RENDER_TARGET_FORMAT format) {
			switch (format)
			{
			case smug::if_render::R8G8B8A8:
				return vk::Format::eR8G8B8A8Unorm;
				break;
			case smug::if_render::R16G16B16A16:
				return vk::Format::eR16G16B16A16Sfloat;
				break;
			case smug::if_render::D32:
				return vk::Format::eD32Sfloat;
				break;
			case smug::if_render::D24S8:
				return vk::Format::eD24UnormS8Uint;
				break;
			}
			return vk::Format::eUndefined;
		}

		void AllocateRenderTarget(int width, int height, int format,const std::string name) {
			globals::g_Gfx->GetFrameBufferManager().AllocRenderTarget(HashString(name), width, height, 1, RTFormatToVKFormat((RENDER_TARGET_FORMAT)format), vk::ImageLayout::eColorAttachmentOptimal);
		}

		void BindRenderTargets(const CScriptArray* renderTargets) {
			for (int i = 0; i < renderTargets->GetSize(); ++i) {
				std::string n = *(std::string*)renderTargets->At(i);
			}
		}

		void Render(uint64_t sortKey) {

		}

		void Dispatch() {

		}

		void InitInterface() {
			
			asIScriptEngine* engine = g_ScriptEngine.GetEngine();
			engine->RegisterEnum("RENDER_TARGET_FORMAT");
			engine->RegisterEnumValue("RENDER_TARGET_FORMAT", DEF_ENUM(R8G8B8A8));
			engine->RegisterEnumValue("RENDER_TARGET_FORMAT", DEF_ENUM(R16G16B16A16));
			engine->RegisterEnumValue("RENDER_TARGET_FORMAT", DEF_ENUM(D32));
			engine->RegisterEnumValue("RENDER_TARGET_FORMAT", DEF_ENUM(D24S8));
			int r = 0;
			r = engine->RegisterGlobalFunction("void AllocateRenderTarget(int width, int height, RENDER_TARGET_FORMAT format, string name)", asFUNCTION(AllocateRenderTarget), asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void BindRenderTargets(const array<string>@ names)", asFUNCTION(BindRenderTargets), asCALL_CDECL); assert(r >= 0);
		}
	}
}