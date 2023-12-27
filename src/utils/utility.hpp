//
// Created by Enzo Crema on 26/12/2023.
//
#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#define NDC_INLINE [[nodiscard]] __attribute__((always_inline)) inline

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