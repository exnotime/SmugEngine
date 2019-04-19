#pragma once
#include "VkPipeline.h"
#include "VkShader.h"
namespace smug {

struct JsonShader {
	eastl::string EntryPoint;
	SHADER_LANGUAGE Lang;
	SHADER_KIND Stage;
	eastl::string SourceFile;
	bool EditorOpen = false;
	bool FileOpened = false;
	char* TextBuffer = nullptr;
	uint32_t TextBufferSize;
};

class PipelineStateEditor {
  public:
	PipelineStateEditor() : m_Device(nullptr) {}
	~PipelineStateEditor() {}
	void Init(VkDevice device);
	void Open(const char* filename);
	void Update();
	PipelineState& GetCurrentPipeline();

  private:
	VkDevice m_Device;
	eastl::vector<JsonShader> m_Shaders;
	PipelineState m_PipelineState;
	eastl::string m_CurrentPipelineFile;
	VkPipelineDepthStencilStateCreateInfo m_DepthStencilState;
	VkPipelineRasterizationStateCreateInfo m_RasterState;
	eastl::vector<VkPipelineColorBlendAttachmentState> m_AttachmentStates;
	int m_ActiveAttachmentEditor;

};
}