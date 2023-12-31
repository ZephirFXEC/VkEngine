//
// Created by Enzo Crema on 21/12/2023.
//

#pragma once

// engine
#include "vkEngineDevice.hpp"

// glm
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "vkEngineSwapChain.hpp"

namespace vke {
class VkEngineModel {
   public:
	struct Vertex {
		glm::vec2 mPosition{};
		glm::vec3 mColor{};

		static std::unique_ptr<std::array<VkVertexInputBindingDescription, 1>> getBindingDescriptions();

		static std::unique_ptr<std::array<VkVertexInputAttributeDescription, 2>> getAttributeDescriptions();
	};

	VkEngineModel(const VkEngineDevice& device, const std::shared_ptr<VkEngineSwapChain>& swapchain,
	              const Vertex* vertices, u32 vCount, const u32* indices, u32 iCount);

	~VkEngineModel();

	VkEngineModel(const VkEngineModel&) = delete;

	VkEngineModel& operator=(const VkEngineModel&) = delete;

	void bind(const VkCommandBuffer* commandBuffer) const;

	void draw(const VkCommandBuffer* commandBuffer) const;

   private:
	template <typename T>
	void createVkBuffer(const T* data, size_t dataSize, VkBufferUsageFlags usage, VkBuffer& buffer,
	                    VmaAllocation& bufferMemory);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
	                  VmaAllocation& bufferMemory) const;

	void destroyBuffer(const DataBuffer& buffer) const;

	void copyBuffer(const VkBuffer* srcBuffer, const VkBuffer* dstBuffer, VkDeviceSize size);

	void createVertexBuffers(const Vertex* vertices, size_t vertexCount);

	void createIndexBuffers(const u32* indices, size_t indexCount);

	DataBuffer mVertexBuffer{};
	DataBuffer mIndexBuffer{};

	VkCommandBuffer pCommandBuffer = VK_NULL_HANDLE;

	u32 mIndexCount = 0;

	const VkEngineDevice& mDevice;

	// note don't access mDevice using the swap chain, since it's
	const std::shared_ptr<VkEngineSwapChain> mSwapChain;
};
}  // namespace vke