#pragma once
#include "RenderInterface.h"
typedef uint64_t ResourceHandle;
namespace smug {

	enum CMD_TYPE {
		CMD_BIND_RENDER_TARGET,
		CMD_RENDER,
		CMD_DISPATCH,
		CMD_COPY,
		CMD_FENCE
	};

	enum RENDER_KEY {
		RENDER_KEY_OPAQUE = 0x1,
		RENDER_KEY_TRANSPARENT = 0x2,
		RENDER_KEY_SHADOW = 0x4,
		RENDER_KEY_RAY_TRACED = 0x8,
		RENDER_KEY_DYNAMIC = 0x10,
		RENDER_KEY_STATIC = 0x20
	};

	struct Cmd {
		CMD_TYPE Type;
		uint32_t Index;
	};
	//bind the list of rendertargets to be rendered to by the next render commands
	struct BindRenderTargetCmd{
		uint32_t* RenderTargets;
		uint32_t RenderTargetCount;
	};
	//A Render cmd renders all geometry for a frame that is captured by the renderkey; //ex rk SHADOWS | COLOR | RAY_TRACED | DYNAMIC | STATIC would render everything while SHADOWS | RAY_TRACED | STATIC would only render the raytraced shadow meshes that are static
	//The sort key includes the depth
	struct RenderCmd {
		ResourceHandle Pipeline;
		RENDER_KEY RenderKey;
		uint64_t SortKey;
		uint32_t StencilRef;
	};
	//render single model with a special shader
	struct RenderModel {
		ResourceHandle Shader;
		ResourceHandle Model;
	};
	//Dispatch executes 1 compute shader pass
	struct DispatchCmd {
		int WorkGroupCountX;
		int WorkGroupCountY;
		int WorkGroupCountZ;
		ResourceHandle Shader;
		int Queue; //queue 0 = GFX, 1 = first Async queue
	};
	//copies one resource to another
	struct CopyCmd {
		ResourceHandle Dst; //resource name
		ResourceHandle Src; //resource name
		uint64_t OffsetDst; //offset into dst
		uint64_t OffsetSrc; //offset into src
		uint64_t Size; // Set to UINT_MAX to copy entire resource
	};
	enum FENCE_CMD {
		FENCE_WAIT,
		FENCE_WRTIE
	};
	struct FenceCmd {
		uint32_t Fence;
		FENCE_CMD cmd;
	};

	class RenderPipeline {
	public:
		RenderPipeline();
		~RenderPipeline();
		void StartRecord();
		void RecordBindRenderTargetCommand(BindRenderTargetCmd cmd);
		void RecordRenderCommand(RenderCmd cmd);
		void RecordDispatchCommand(DispatchCmd cmd);
		void RecordCopyCommand(CopyCmd cmd);
		void RecordFenceCommand(FenceCmd cmd);
		void EndRecord();

		void StartPlayback();
		const Cmd&					GetNextCommand();
		const BindRenderTargetCmd&	GetBindRenderTargetCommand(uint32_t index);
		const RenderCmd&			GetRenderCommand(uint32_t index);
		const DispatchCmd&			GetDispatchCommand(uint32_t index);
		const CopyCmd&				GetCopyCommand(uint32_t index);
		const FenceCmd&				GetFenceCommand(uint32_t index);
		void EndPlayback();
	private:
		std::vector<Cmd> m_Commands;
		std::vector<BindRenderTargetCmd> m_RenderTargetCommands;
		std::vector<RenderCmd> m_RenderCommands;
		std::vector<DispatchCmd> m_DispatchCommands;
		std::vector<CopyCmd> m_CopyCommands;
		std::vector<FenceCmd> m_FenceCommands;
		bool m_Recording;
		bool m_Playback;
		uint32_t m_CmdIndex;
	};
}