//
// Created by Enzo Crema on 26/12/2023.
//
#pragma once

#include <fmt/core.h>
#include <vk_mem_alloc.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan.h>

#include <deque>
#include <functional>
#include <ranges>
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

#define GETTERS(type, name, var) \
	NDC_INLINE const type& get##name() const { return var; }

#define NDC_FINLINE [[nodiscard]] __attribute__((always_inline)) inline
#define NDC_INLINE [[nodiscard]] inline

// User defined types
static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;


struct DeletionQueue {
	std::deque<std::function<void()>> mQueue{};
	void push_function(std::function<void()>&& function) { mQueue.push_back(std::move(function)); }

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (const auto& it : std::ranges::reverse_view(mQueue)) {
			it();
		}

		mQueue.clear();
	}
};


struct DataBuffer {
	VkBuffer pDataBuffer = VK_NULL_HANDLE;
	VmaAllocation pDataBufferMemory = VK_NULL_HANDLE;
};


struct Shader {
	VkShaderModule pVertShaderModule = VK_NULL_HANDLE;
	VkShaderModule pFragShaderModule = VK_NULL_HANDLE;
};



struct NO_COPY_NOR_MOVE {
	NO_COPY_NOR_MOVE() = default;
	NO_COPY_NOR_MOVE(const NO_COPY_NOR_MOVE&) = delete;
	NO_COPY_NOR_MOVE& operator=(const NO_COPY_NOR_MOVE&) = delete;
	NO_COPY_NOR_MOVE(NO_COPY_NOR_MOVE&&) = delete;
	NO_COPY_NOR_MOVE& operator=(NO_COPY_NOR_MOVE&&) = delete;
};


struct FrameData : NO_COPY_NOR_MOVE {
	VkCommandPool pCommandPool = VK_NULL_HANDLE;
	VkCommandBuffer pCommandBuffer = VK_NULL_HANDLE;
};


struct SyncPrimitives : NO_COPY_NOR_MOVE {
	VkSemaphore* ppImageAvailableSemaphores = nullptr;  // Semaphores for image availability
	VkSemaphore* ppRenderFinishedSemaphores = nullptr;  // Semaphores for render finishing
	VkFence* ppInFlightFences = nullptr;                // Fences for in-flight operations
	VkFence* ppInFlightImages = nullptr;                // Fences for in-flight images
};


struct VkImageRessource : NO_COPY_NOR_MOVE {
	VkImage* ppImages = nullptr;
	VkImageView* ppImageViews = nullptr;
	VmaAllocation* ppImageMemorys = nullptr;
};


struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR mCapabilities{};
	std::vector<VkSurfaceFormatKHR> mFormats{};
	std::vector<VkPresentModeKHR> mPresentModes{};
};


struct QueueFamilyIndices {
	std::optional<uint32_t> mGraphicsFamily;
	std::optional<uint32_t> mPresentFamily;

	[[nodiscard]] bool isComplete() const { return mGraphicsFamily.has_value() && mPresentFamily.has_value(); }
};


struct AllocatedImage {
	VkImage pImage;
	VkImageView pImageView;
	VmaAllocation pAllocation;
	VkExtent3D mImageExtent;
	VkFormat mImageFormat;
};