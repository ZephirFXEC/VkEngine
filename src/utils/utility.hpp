//
// Created by Enzo Crema on 26/12/2023.
//
#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

#define NDC_FINLINE [[nodiscard]] __attribute__((always_inline)) inline
#define NDC_INLINE [[nodiscard]] inline

// constants for the application
#define VMA

#ifdef VMA
#define USE_VMA
using Alloc = VmaAllocation;
#else
using Alloc = VkDeviceMemory;
#endif

struct DataBuffer {
	VkBuffer pDataBuffer = VK_NULL_HANDLE;
	Alloc pDataBufferMemory = VK_NULL_HANDLE;
};

struct Shader {
	VkShaderModule pVertShaderModule = VK_NULL_HANDLE;
	VkShaderModule pFragShaderModule = VK_NULL_HANDLE;
};

struct FrameData {
	VkCommandPool pCommandPool = VK_NULL_HANDLE;
	VkCommandBuffer pCommandBuffer = VK_NULL_HANDLE;
};