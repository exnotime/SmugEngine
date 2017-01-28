#pragma once
#include <vulkan/vulkan.hpp>

#define BUFFER_COUNT 2

struct VulkanContext {
	vk::Instance Instance;
	vk::Device Device;
	vk::PhysicalDevice PhysicalDevice;
	vk::Queue GFXQueue;
	uint32_t FrameIndex;
};

struct VulkanSwapChain {
	vk::SurfaceKHR Surface;
	vk::SwapchainKHR SwapChain;
	vk::Image Images[BUFFER_COUNT];
	vk::ImageView ImageViews[BUFFER_COUNT];
	vk::Framebuffer FrameBuffers[BUFFER_COUNT];
};

struct CommandBuffer {
	vk::CommandBuffer CmdBuffer;
	vk::CommandPool CmdPool[BUFFER_COUNT];
};