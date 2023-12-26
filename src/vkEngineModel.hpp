//
// Created by Enzo Crema on 21/12/2023.
//

#pragma once

// engine
#include "vkEngineDevice.hpp"

// glm
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

constexpr static bool USE_VMA = true;
using Alloc = std::conditional_t <USE_VMA, VmaAllocation, VkDeviceMemory>;

namespace vke {
class VkEngineModel {
public:
	struct Vertex {
		glm::vec2 mPosition{};
		glm::vec3 mColor{};

		static VkVertexInputBindingDescription* getBindingDescriptions();

		static VkVertexInputAttributeDescription* getAttributeDescriptions();
	};

	VkEngineModel(VkEngineDevice& device, const Vertex* vertices, uint32_t vCount, const uint32_t* indices,
	              uint32_t iCount);

	~VkEngineModel();

	VkEngineModel(const VkEngineModel&) = delete;

	VkEngineModel& operator=(const VkEngineModel&) = delete;

	void bind(VkCommandBuffer commandBuffer) const;

	void draw(VkCommandBuffer commandBuffer) const;

private:
	template <typename MemAlloc>
	struct DataBuffer {
		VkBuffer pDataBuffer = VK_NULL_HANDLE;
		MemAlloc pDataBufferMemory = VK_NULL_HANDLE;
	};

	template <typename T, typename MemAlloc>
	void createVkBuffer(const T* data, size_t dataSize, VkBufferUsageFlags usage, VkBuffer& buffer,
	                    MemAlloc& bufferMemory) const;

	template <typename MemAlloc>
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
	                  MemAlloc& bufferMemory) const;

	template <typename MemAlloc>
	void destroyBuffer(const DataBuffer <MemAlloc>& buffer) const;

	void createVertexBuffers(const Vertex* vertices, size_t vertexCount);

	void createIndexBuffers(const uint32_t* indices, size_t indexCount);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

	DataBuffer <Alloc> mVertexBuffer{};
	DataBuffer <Alloc> mIndexBuffer{};

	uint32_t mIndexCount = 0;

	VkEngineDevice& mDevice;
};
} // namespace vke
