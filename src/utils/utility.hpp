//
// Created by Enzo Crema on 26/12/2023.
//
#pragma once

#include <fmt/core.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include <array>
#include <deque>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <vector>
#include <set>

#ifndef NDEBUG
#define VK_CHECK(x)                                                          \
	do {                                                                     \
		VkResult err = x;                                                    \
		if (err) {                                                           \
			fmt::println("Detected Vulkan error: {}", string_VkResult(err)); \
			abort();                                                         \
		}                                                                    \
	} while (0)
#else
#define VK_CHECK(x) x
#endif

#define NDC_FINLINE [[nodiscard]] __attribute__((always_inline)) inline
#define NDC_INLINE [[nodiscard]] inline

// User defined types
#define VMA
static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;



// Definitions
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