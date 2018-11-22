#include "RenderPipeline.h"
using namespace smug;

RenderPipeline::RenderPipeline() {}
RenderPipeline::~RenderPipeline() {}

void RenderPipeline::StartRecord() {
	m_Recording = true;
}

void RenderPipeline::RecordBindRenderTargetCommand(BindRenderTargetCmd cmd){
	Cmd c;
	c.Index = (uint32_t)m_RenderTargetCommands.size();
	c.Type = CMD_BIND_RENDER_TARGET;
	m_RenderTargetCommands.push_back(cmd);
	m_Commands.push_back(c);
}

void RenderPipeline::RecordRenderCommand(RenderCmd cmd){
	Cmd c;
	c.Index = (uint32_t)m_RenderCommands.size();
	c.Type = CMD_RENDER;
	m_RenderCommands.push_back(cmd);
	m_Commands.push_back(c);
}

void RenderPipeline::RecordDispatchCommand(DispatchCmd cmd){
	Cmd c;
	c.Index = (uint32_t)m_DispatchCommands.size();
	c.Type = CMD_DISPATCH;
	m_DispatchCommands.push_back(cmd);
	m_Commands.push_back(c);
}

void RenderPipeline::RecordCopyCommand(CopyCmd cmd){
	Cmd c;
	c.Index = (uint32_t)m_CopyCommands.size();
	c.Type = CMD_COPY;
	m_CopyCommands.push_back(cmd);
	m_Commands.push_back(c);
}

void RenderPipeline::RecordFenceCommand(FenceCmd cmd){
	Cmd c;
	c.Index = (uint32_t)m_FenceCommands.size();
	c.Type = CMD_FENCE;
	m_FenceCommands.push_back(cmd);
	m_Commands.push_back(c);
}

void RenderPipeline::EndRecord(){
	m_Recording = false;
}

void RenderPipeline::StartPlayback(){
	m_CmdIndex = 0;
	m_Playback = true;
}

const Cmd& RenderPipeline::GetNextCommand(){
	return m_Commands[m_CmdIndex++];
}

const BindRenderTargetCmd& RenderPipeline::GetBindRenderTargetCommand(uint32_t index){
	return m_RenderTargetCommands[index];
}

const RenderCmd& RenderPipeline::GetRenderCommand(uint32_t index){
	return m_RenderCommands[index];
}

const DispatchCmd& RenderPipeline::GetDispatchCommand(uint32_t index){
	return m_DispatchCommands[index];
}

const CopyCmd& RenderPipeline::GetCopyCommand(uint32_t index){
	return m_CopyCommands[index];
}

const FenceCmd&	RenderPipeline::GetFenceCommand(uint32_t index){
	return m_FenceCommands[index];
}

void RenderPipeline::EndPlayback(){
	m_Playback = false;
}