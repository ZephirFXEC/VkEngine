//
// Created by Enzo Crema on 21/12/2023.
//

#pragma once

#include "utils/utility.hpp"
// engine
#include "vkEngineDevice.hpp"

// glm
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vke {
class VkEngineModel {
   public:
	struct Vertex {
		glm::vec2 mPosition{};
		glm::vec3 mColor{};

		static std::unique_ptr<std::array<VkVertexInputBindingDescription, 1>> getBindingDescriptions();

		static std::unique_ptr<std::array<VkVertexInputAttributeDescription, 2>> getAttributeDescriptions();
	};

	VkEngineModel(VkEngineDevice& device, const Vertex* vertices, uint32_t vCount, const uint32_t* indices,
	              uint32_t iCount);

	~VkEngineModel();

	VkEngineModel(const VkEngineModel&) = delete;

	VkEngineModel& operator=(const VkEngineModel&) = delete;

	void bind(const VkCommandBuffer* commandBuffer) const;

	void draw(const VkCommandBuffer* commandBuffer) const;

   private:
	template <typename T>
	void createVkBuffer(const T* data, size_t dataSize, VkBufferUsageFlags usage, VkBuffer& buffer,
	                    Alloc& bufferMemory);

	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
	                  Alloc& bufferMemory) const;

	void destroyBuffer(const DataBuffer& buffer) const;

	void createVertexBuffers(const Vertex* vertices, size_t vertexCount);

	void createIndexBuffers(const uint32_t* indices, size_t indexCount);

	void copyBuffer(const VkBuffer* srcBuffer, const VkBuffer* dstBuffer, VkDeviceSize size);

	DataBuffer mVertexBuffer{};
	DataBuffer mIndexBuffer{};

	VkCommandBuffer pCommandBuffer = VK_NULL_HANDLE;

	uint32_t mIndexCount = 0;

	const VkEngineDevice& mDevice;
};
}  // namespace vke