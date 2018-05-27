#pragma once
#include "VkPipeline.h"
#include "VkShader.h"
namespace smug {

	struct JsonShader {
		std::string EntryPoint;
		SHADER_LANGUAGE Lang;
		SHADER_KIND Stage;
		std::string SourceFile;
		bool EditorOpen = false;
		bool FileOpened = false;
		char* TextBuffer = nullptr;
		uint32_t TextBufferSize;
	};

	class PipelineStateEditor {
	public:
		PipelineStateEditor() : m_Device(nullptr){}
		~PipelineStateEditor(){}
		void Init(VkDevice device);
		void Open(const char* filename);
		void Update();
		PipelineState& GetCurrentPipeline();

	private:
		VkDevice m_Device;
		std::vector<JsonShader> m_Shaders;
		PipelineState m_PipelineState;
		std::string m_CurrentPipelineFile;
		vk::PipelineDepthStencilStateCreateInfo m_DepthStencilState;
		vk::PipelineRasterizationStateCreateInfo m_RasterState;
		std::vector<vk::PipelineColorBlendAttachmentState> m_AttachmentStates;
		int m_ActiveAttachmentEditor;

	};
}