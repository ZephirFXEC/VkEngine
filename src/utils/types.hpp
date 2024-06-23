//
// Created by Enzo Crema on 26/12/2023.
//

#pragma once

#include <vk_mem_alloc.h>

#include <functional>
#include <optional>
#include <vector>


using u8 = uint8_t;
using i8 = int8_t;
using u16 = uint16_t;
using i16 = int16_t;
using u32 = uint32_t;
using i32 = int32_t;
using u64 = uint64_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;


struct DeletionQueue {
	using DeletorFunc = void (*)(void*);

	struct Deletor {
		void* callable;
		DeletorFunc call;

		Deletor(void* c, const DeletorFunc f) : callable(c), call(f) {}
	};

	template <typename Callable>
	void push_function(Callable&& function) {
		deletors.emplace_back(new std::decay_t<Callable>(std::forward<Callable>(function)), [](void* c) {
			auto callable = static_cast<std::decay_t<Callable>*>(c);
			(*callable)();
			delete callable;
		});
	}

	void flush() {
		for (auto it = deletors.rbegin(); it != deletors.rend(); ++it) {
			it->call(it->callable);
		}
		deletors.clear();
	}

   private:
	std::vector<Deletor> deletors;
};


// User defined types
static constexpr u8 MAX_FRAMES_IN_FLIGHT = 2;

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

	~NO_COPY_NOR_MOVE() = default;
};


struct SyncPrimitives : NO_COPY_NOR_MOVE {
	VkSemaphore ppImageAvailableSemaphores[MAX_FRAMES_IN_FLIGHT];  // Semaphores for image availability
	VkSemaphore ppRenderFinishedSemaphores[MAX_FRAMES_IN_FLIGHT];  // Semaphores for render finishing
	VkFence ppInFlightFences[MAX_FRAMES_IN_FLIGHT];                // Fences for in-flight operations
	VkFence* ppInFlightImages = nullptr;
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
	std::optional<u32> mGraphicsFamily{};
	std::optional<u32> mPresentFamily{};

	[[nodiscard]] bool isComplete() const { return mGraphicsFamily.has_value() && mPresentFamily.has_value(); }
};
