#include "PipelineStateEditor.h"
#include <json.hpp>
#include "Imgui/imgui.h"
#include <sstream>
#include <fstream>
#include "VkShader.h"
#include "PipelineCommon.h"

using namespace smug;
void PipelineStateEditor::Init(VkDevice device) {
	m_Device = device;

	m_DepthStencilState = GetDefaultDepthStencilstate();
	m_RasterState = GetDefaultRasterState();
}

void FillshaderTextBuffer(const char* file, char** buffer, uint32_t* bufferSize) {
	FILE* f = fopen(file, "rb");
	if (f) {
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		fseek(f, 0, SEEK_SET);
		if (*buffer) free(*buffer);
		*buffer = (char*)malloc(size + 4096);
		*bufferSize = size + 4096;
		fread(*buffer, sizeof(char), *bufferSize, f);
	}
}

void PipelineStateEditor::Open(const char* filename) {
	using namespace nlohmann;
	m_CurrentPipelineFile = filename;
	//open json pipeline state
	std::ifstream fin(filename);
	if(!fin.is_open()) {
		return;
	}
	json root;
	try {
		root << fin;
	} catch (const std::exception& e) {
		printf("%s\n", e.what());
		return;
	}
	fin.close();

	//Find shaders
	if (root.find("Shaders") != root.end()) {
		json shaders = root["Shaders"];

		const char* shader_types[] = { "Vertex", "Fragment", "Geometry", "EVALUATION", "Control", "Compute" };
		for (int i = 0; i < SHADER_KIND::COMPUTE + 1; ++i) {
			if (shaders.find(shader_types[i]) != shaders.end()) {
				json shader = shaders[shader_types[i]];
				JsonShader shaderEntry;

				if (shader.find("EntryPoint") != shader.end()) {
					std::string entryPoint = shader["EntryPoint"];
					shaderEntry.EntryPoint = entryPoint.c_str();
				} else {
					shaderEntry.EntryPoint = "main";
				}
				if (shader.find("Language") != shader.end()) {
					if (shader["Language"] == "GLSL") {
						shaderEntry.Lang = SHADER_LANGUAGE::GLSL;
					} else if (shader["Language"] == "HLSL") {
						shaderEntry.Lang = SHADER_LANGUAGE::HLSL;
					}
				} else {
					shaderEntry.Lang = SHADER_LANGUAGE::GLSL;
				}
				shaderEntry.Stage = (SHADER_KIND)i;
				std::string source = shader["Source"];
				shaderEntry.SourceFile = source.c_str();
				m_Shaders.push_back(shaderEntry);
			}
		}
	}

	m_AttachmentStates.push_back(GetDefaultColorBlendAttachmentState());
	m_ActiveAttachmentEditor = -1;
}

void RenderTextEditor(const char* name, char* text, uint32_t bufferSize) {
	ImVec2 size = { 12 * 40, 12 * 60 };
	ImGui::InputTextMultiline("", text, bufferSize, size); //current shader
}

void RenderDepthStencilStateEditor(VkPipelineDepthStencilStateCreateInfo& depthInfo) {
	//ImGui::LabelText("", "Depth");
	//ImGui::Checkbox("Enable depth test", (bool*)&depthInfo.depthTestEnable);
	//eastl::vector<const char*> compareOPslabels;
	//int currentIndex = 0;
	//int i = 0;
	//for (auto& comp : ToCompareOp) {
	//	compareOPslabels.push_back(comp.first.c_str());
	//	if (depthInfo.depthCompareOp == comp.second)
	//		currentIndex = i;
	//	i++;
	//}
	//ImGui::Combo("Compare op", &currentIndex, compareOPslabels.data(), (uint32_t)compareOPslabels.size());
	//depthInfo.depthCompareOp = ToCompareOp[compareOPslabels[currentIndex]];
	//ImGui::Checkbox("Enable depth write", (bool*)&depthInfo.depthWriteEnable);
	//ImGui::Checkbox("Enable depth bounds", (bool*)&depthInfo.depthBoundsTestEnable);
	//ImGui::InputFloat("Min depth bounds", &depthInfo.minDepthBounds);
	//ImGui::InputFloat("Max depth bounds", &depthInfo.minDepthBounds);
	//ImGui::LabelText("", "Stencil");
	//ImGui::Checkbox("Enable stencil test", (bool*)&depthInfo.stencilTestEnable);
	//eastl::vector<const char*> stencilOpLabels;
	//int frontIndex = 0;
	//int backIndex = 0;
	//i = 0;
	//for (auto& op : ToStencilOp) {
	//	stencilOpLabels.push_back(op.first.c_str());
	//	if (depthInfo.front == op.second)
	//		frontIndex = i;
	//	if (depthInfo.back == op.second)
	//		backIndex = i;
	//	i++;
	//}
	//ImGui::Combo("Front Stencil op", &frontIndex, stencilOpLabels.data(), (uint32_t)stencilOpLabels.size());
	//ImGui::Combo("Back Stencil op", &backIndex, stencilOpLabels.data(), (uint32_t)stencilOpLabels.size());
	//depthInfo.front = ToStencilOp[stencilOpLabels[frontIndex]];
	//depthInfo.back = ToStencilOp[stencilOpLabels[backIndex]];
}

void RenderRasterizerStateEditor(VkPipelineRasterizationStateCreateInfo& rasterInfo) {
	//cull mode
	eastl::vector<const char*> cullModelabels;
	int cullIndex = 0;
	int i = 0;
	for (auto& cull : ToCullMode) {
		cullModelabels.push_back(cull.first.c_str());
		if (rasterInfo.cullMode == cull.second)
			cullIndex = i;
		i++;
	}
	ImGui::Combo("Cull mode", &cullIndex, cullModelabels.data(), (uint32_t)cullModelabels.size());
	rasterInfo.cullMode = ToCullMode[cullModelabels[cullIndex]];
	//front face
	eastl::vector<const char*> frontFacelabels;
	int frontFaceIndex = 0;
	i = 0;
	for (auto& face : ToFrontFace) {
		frontFacelabels.push_back(face.first.c_str());
		if (rasterInfo.frontFace == face.second)
			frontFaceIndex = i;
		i++;
	}
	ImGui::Combo("Front face", &frontFaceIndex, frontFacelabels.data(), (uint32_t)frontFacelabels.size());
	rasterInfo.frontFace = ToFrontFace[frontFacelabels[frontFaceIndex]];
	//polygon mode
	eastl::vector<const char*> polygonModeLabels;
	int polygonIndex = 0;
	i = 0;
	for (auto& mode : ToPolygonMode) {
		polygonModeLabels.push_back(mode.first.c_str());
		if (rasterInfo.polygonMode == mode.second)
			polygonIndex = i;
		i++;
	}
	ImGui::Combo("Polygon mode", &polygonIndex, polygonModeLabels.data(), (uint32_t)polygonModeLabels.size());
	rasterInfo.polygonMode = ToPolygonMode[polygonModeLabels[polygonIndex]];
	ImGui::Checkbox("Enable discard", (bool*)&rasterInfo.rasterizerDiscardEnable);
	ImGui::InputFloat("LineWidth", &rasterInfo.lineWidth);
	ImGui::Checkbox("Enable depth bias", (bool*)&rasterInfo.depthBiasEnable);
	ImGui::InputFloat("Depth bias slope factor", &rasterInfo.depthBiasSlopeFactor);
	ImGui::InputFloat("Depth bias constant factor", &rasterInfo.depthBiasConstantFactor);
	ImGui::InputFloat("Depth bias clamp", &rasterInfo.depthBiasClamp);
}

int RenderAttachmentStatesChooser(eastl::vector<VkPipelineColorBlendAttachmentState>& states, int activeAttachment) {
	uint32_t stateCount = (uint32_t)states.size();
	for (uint32_t i = 0; i < stateCount; i++) {
		ImGui::Text("Attachment %d", i);
		ImGui::SameLine();
		if (ImGui::SmallButton("Edit")) {
			if (activeAttachment == i) {
				activeAttachment = -1;
			} else {
				activeAttachment = i;
			}
		}
	}
	if (ImGui::Button("Add")) {
		states.push_back(GetDefaultColorBlendAttachmentState());
	}

	return activeAttachment;
}

void RenderAttachmentStateEditor(VkPipelineColorBlendAttachmentState& attachmentState) {
	ImGui::Checkbox("Enable blend", (bool*)&attachmentState.blendEnable);
	eastl::vector<const char*> blendOplabels;
	int alphaOpIndex = 0;
	int colorOpIndex = 0;
	int i = 0;
	for (auto& op : ToBlendOp) {
		blendOplabels.push_back(op.first.c_str());
		if (attachmentState.alphaBlendOp == op.second)
			alphaOpIndex = i;
		if (attachmentState.colorBlendOp == op.second)
			colorOpIndex = i;
		i++;
	}

	eastl::vector<const char*> blendFactorLabels;
	int srcAlphaIndex = 0;
	int dstAlphaIndex = 0;
	int srcColorIndex = 0;
	int dstColorIndex = 0;
	i = 0;
	for (auto& blend : ToBlendFactor) {
		blendFactorLabels.push_back(blend.first.c_str());
		if (attachmentState.srcAlphaBlendFactor == blend.second)
			srcAlphaIndex = i;
		if (attachmentState.dstAlphaBlendFactor == blend.second)
			dstAlphaIndex = i;
		if (attachmentState.srcColorBlendFactor == blend.second)
			srcColorIndex = i;
		if (attachmentState.dstColorBlendFactor == blend.second)
			dstColorIndex = i;
		i++;
	}


	ImGui::Combo("Alpha Blend Op", &alphaOpIndex, blendOplabels.data(), (uint32_t)blendOplabels.size());
	attachmentState.alphaBlendOp = ToBlendOp[blendOplabels[alphaOpIndex]];
	ImGui::Combo("Src alpha blend factor ", &srcAlphaIndex, blendFactorLabels.data(), (uint32_t)blendFactorLabels.size());
	attachmentState.srcAlphaBlendFactor = ToBlendFactor[blendFactorLabels[srcAlphaIndex]];
	ImGui::Combo("Dst alpha blend factor ", &dstAlphaIndex, blendFactorLabels.data(), (uint32_t)blendFactorLabels.size());
	attachmentState.dstAlphaBlendFactor = ToBlendFactor[blendFactorLabels[dstAlphaIndex]];

	ImGui::Combo("Color Blend Op", &colorOpIndex, blendOplabels.data(), (uint32_t)blendOplabels.size());
	attachmentState.colorBlendOp = ToBlendOp[blendOplabels[colorOpIndex]];
	ImGui::Combo("Src color blend factor ", &srcColorIndex, blendFactorLabels.data(), (uint32_t)blendFactorLabels.size());
	attachmentState.srcColorBlendFactor = ToBlendFactor[blendFactorLabels[srcColorIndex]];
	ImGui::Combo("Dst color blend factor ", &dstColorIndex, blendFactorLabels.data(), (uint32_t)blendFactorLabels.size());
	attachmentState.dstColorBlendFactor = ToBlendFactor[blendFactorLabels[dstColorIndex]];

}

void PipelineStateEditor::Update() {
	//ImGui::ShowDemoWindow();
	//main window
	//shaders
	const static eastl::vector<const char*> shaderStrings = { "Vertex", "Fragment", "Geometry", "Evaluation", "Control", "Compute" };
	bool anyShaderOpen = false;
	ImGui::Begin("PipelineState Editor");
	ImGui::BeginGroup();
	ImGui::Text("Shaders");
	for (auto& shader : m_Shaders) {
		if (ImGui::Button(shaderStrings[shader.Stage])) {
			shader.EditorOpen = !shader.EditorOpen;
		}
	}
	ImGui::EndGroup();
	//render states
	ImGui::BeginGroup();
	ImGui::Text("States");
	static bool depthStateEditor = false;
	if (ImGui::Button("DepthStencilState")) {
		depthStateEditor = !depthStateEditor;
	}
	static bool rasterStateEditor = false;
	if (ImGui::Button("RasterizerState")) {
		rasterStateEditor = !rasterStateEditor;
	}

	ImGui::EndGroup();
	ImGui::Text("Attachments");
	m_ActiveAttachmentEditor = RenderAttachmentStatesChooser(m_AttachmentStates, m_ActiveAttachmentEditor);
	ImGui::End();
	//shader module Editor
	for (auto& shader : m_Shaders) {
		if (shader.EditorOpen) {
			if (!shader.FileOpened) {
				FillshaderTextBuffer(shader.SourceFile.c_str(), &shader.TextBuffer, &shader.TextBufferSize);
				shader.FileOpened = true;
			}
			ImGui::Begin(shaderStrings[shader.Stage]);
			RenderTextEditor(shaderStrings[shader.Stage], shader.TextBuffer, shader.TextBufferSize);
			ImGui::End();
		}
	}
	//DepthStencilState
	if (depthStateEditor) {
		ImGui::Begin("DepthStencilStateEditor");
		RenderDepthStencilStateEditor(m_DepthStencilState);
		if (ImGui::Button("Close"))
			depthStateEditor = !depthStateEditor;
		ImGui::End();
	}

	//RasterizerState
	if (rasterStateEditor) {
		ImGui::Begin("RasterizerStateEditor");
		RenderRasterizerStateEditor(m_RasterState);
		if (ImGui::Button("Close"))
			rasterStateEditor = !rasterStateEditor;
		ImGui::End();
	}

	if (m_ActiveAttachmentEditor != -1) {
		ImGui::Begin("AttachmentBlendStateEditor");
		RenderAttachmentStateEditor(m_AttachmentStates[m_ActiveAttachmentEditor]);
		if (ImGui::Button("Close"))
			m_ActiveAttachmentEditor = -1;
		ImGui::End();
	}

}