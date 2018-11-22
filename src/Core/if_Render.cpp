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

		enum RENDER_QUEUE {
			RENDER_QUEUE_GFX,
			RENDER_QUEUE_COMPUTE0,
			RENDER_QUEUE_COMPUTE1,
			RENDER_QUEUE_COMPUTE2,
			RENDER_QUEUE_COMPUTE3,
			RENDER_QUEUE_COPY
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

		void AllocateRenderTarget(int width, int height, int format, const std::string name) {
			globals::g_Gfx->GetFrameBufferManager().AllocRenderTarget(HashString(name), width, height, 1, RTFormatToVKFormat((RENDER_TARGET_FORMAT)format), vk::ImageLayout::eColorAttachmentOptimal);
		}

		enum BUFFER_USAGE {
			BUFFER_USAGE_UNIFORM = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			BUFFER_USAGE_STORAGE = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
			BUFFER_USAGE_VERTEX = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			BUFFER_USAGE_INDEX = VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			BUFFER_USAGE_INDIRECT = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT,
			BUFFER_USAGE_TRANSFER_DST = VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			BUFFER_USAGE_TRANSFER_SRC = VK_BUFFER_USAGE_TRANSFER_SRC_BIT
		};

		void AllocateBuffer(uint64_t size, int usage, const std::string name) {
			ResourceHandle handle = CreateHandle(HashString(name.c_str()), RT_BUFFER);
			globals::g_Gfx->GetResourceHandler().AllocateBuffer(size, usage, handle);
		}

		uint64_t LoadShader(std::string name) {
			ResourceHandle h = g_AssetLoader.LoadAsset(name.c_str());
			return h;
		}

		void BindRenderTargets(const CScriptArray* renderTargets) {
			BindRenderTargetCmd rtCmd;
			rtCmd.RenderTargetCount = renderTargets->GetSize();
			rtCmd.RenderTargets = (uint32_t*)malloc(sizeof(uint32_t) * rtCmd.RenderTargetCount);
			for (int i = 0; i < renderTargets->GetSize(); ++i) {
				rtCmd.RenderTargets[i] = HashString(*(std::string*)renderTargets->At(i));
			}
			globals::g_Gfx->GetRenderPipeline().RecordBindRenderTargetCommand(rtCmd);
			renderTargets->Release();
		}

		void Render(int key, uint32_t stencilRef) {
			RenderCmd cmd;
			cmd.RenderKey = (RENDER_KEY)key;
			cmd.SortKey = UINT64_MAX;
			cmd.StencilRef = stencilRef;
			globals::g_Gfx->GetRenderPipeline().RecordRenderCommand(cmd);
		}

		void Dispatch(const uint64_t shader, int workGroupX, int workGroupY, int workGroupZ, int queue) {
			DispatchCmd cmd;
			cmd.Queue = queue;
			cmd.Shader = shader;
			cmd.WorkGroupCountX = workGroupX;
			cmd.WorkGroupCountY = workGroupY;
			cmd.WorkGroupCountZ = workGroupZ;
			globals::g_Gfx->GetRenderPipeline().RecordDispatchCommand(cmd);
		}

		void Copy(const std::string name, const std::string dst, const std::string src, uint64_t offsetDst, uint64_t offsetSrc, uint64_t size) {

		}

		void Fence(const std::string name, const std::string fence, FENCE_CMD cmd) {

		}

		void BeginRenderPass(const std::string name) {
			globals::g_Gfx->GetRenderPipeline().StartRecord();
		}

		void EndRenderPass() {
			globals::g_Gfx->GetRenderPipeline().EndRecord();
		}

		void InitInterface() {
			
			asIScriptEngine* engine = g_ScriptEngine.GetEngine();
			engine->RegisterEnum("RENDER_TARGET_FORMAT");
			engine->RegisterEnumValue("RENDER_TARGET_FORMAT", DEF_ENUM(R8G8B8A8));
			engine->RegisterEnumValue("RENDER_TARGET_FORMAT", DEF_ENUM(R16G16B16A16));
			engine->RegisterEnumValue("RENDER_TARGET_FORMAT", DEF_ENUM(D32));
			engine->RegisterEnumValue("RENDER_TARGET_FORMAT", DEF_ENUM(D24S8));
			engine->RegisterEnum("RENDER_KEY");
			engine->RegisterEnumValue("RENDER_KEY", DEF_ENUM(RENDER_KEY_OPAQUE));
			engine->RegisterEnumValue("RENDER_KEY", DEF_ENUM(RENDER_KEY_TRANSPARENT));
			engine->RegisterEnumValue("RENDER_KEY", DEF_ENUM(RENDER_KEY_SHADOW));
			engine->RegisterEnumValue("RENDER_KEY", DEF_ENUM(RENDER_KEY_RAY_TRACED));
			engine->RegisterEnumValue("RENDER_KEY", DEF_ENUM(RENDER_KEY_DYNAMIC));
			engine->RegisterEnumValue("RENDER_KEY", DEF_ENUM(RENDER_KEY_STATIC));
			engine->RegisterEnum("FENCE_CMD");
			engine->RegisterEnumValue("FENCE_CMD", DEF_ENUM(FENCE_WAIT));
			engine->RegisterEnumValue("FENCE_CMD", DEF_ENUM(FENCE_WRTIE));
			engine->RegisterEnum("RENDER_QUEUE");
			engine->RegisterEnumValue("RENDER_QUEUE", DEF_ENUM(RENDER_QUEUE_GFX));
			engine->RegisterEnumValue("RENDER_QUEUE", DEF_ENUM(RENDER_QUEUE_COMPUTE0));
			engine->RegisterEnumValue("RENDER_QUEUE", DEF_ENUM(RENDER_QUEUE_COMPUTE1));
			engine->RegisterEnumValue("RENDER_QUEUE", DEF_ENUM(RENDER_QUEUE_COMPUTE2));
			engine->RegisterEnumValue("RENDER_QUEUE", DEF_ENUM(RENDER_QUEUE_COMPUTE3));
			engine->RegisterEnumValue("RENDER_QUEUE", DEF_ENUM(RENDER_QUEUE_COPY));
			engine->RegisterEnum("BUFFER_USAGE");
			engine->RegisterEnumValue("BUFFER_USAGE", DEF_ENUM(BUFFER_USAGE_UNIFORM));
			engine->RegisterEnumValue("BUFFER_USAGE", DEF_ENUM(BUFFER_USAGE_STORAGE));
			engine->RegisterEnumValue("BUFFER_USAGE", DEF_ENUM(BUFFER_USAGE_VERTEX));
			engine->RegisterEnumValue("BUFFER_USAGE", DEF_ENUM(BUFFER_USAGE_INDEX));
			engine->RegisterEnumValue("BUFFER_USAGE", DEF_ENUM(BUFFER_USAGE_INDIRECT));
			engine->RegisterEnumValue("BUFFER_USAGE", DEF_ENUM(BUFFER_USAGE_TRANSFER_DST));
			engine->RegisterEnumValue("BUFFER_USAGE", DEF_ENUM(BUFFER_USAGE_TRANSFER_SRC));

			int r = 0;
			r = engine->RegisterGlobalFunction("void AllocateRenderTarget(int width, int height, RENDER_TARGET_FORMAT format, string name)", asFUNCTION(AllocateRenderTarget), asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void AllocateBuffer(uint64 size, int usage, string name)", asFUNCTION(AllocateBuffer), asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void BeginRenderPass(const string name)", asFUNCTION(BeginRenderPass), asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void EndRenderPass()", asFUNCTION(EndRenderPass), asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("uint64 LoadShader(string name)", asFUNCTION(LoadShader), asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void Render(int key, uint stencilRef)", asFUNCTION(Render), asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void Dispatch(const uint64 shader, int workGroupX, int workGroupY, int workGroupZ, int queue)", asFUNCTION(Dispatch), asCALL_CDECL); assert(r >= 0);
			r = engine->RegisterGlobalFunction("void BindRenderTargets(const array<string>@ names)", asFUNCTION(BindRenderTargets), asCALL_CDECL); assert(r >= 0);
		}
	}
}