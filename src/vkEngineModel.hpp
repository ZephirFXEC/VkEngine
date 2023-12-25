//
// Created by Enzo Crema on 21/12/2023.
//

#ifndef VKENGINEMODEL_HPP
#define VKENGINEMODEL_HPP

// engine
#include "vkEngineDevice.hpp"

// glm
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace vke {
class VkEngineModel {
   public:
	struct Vertex {
		glm::vec2 mPosition{};
		glm::vec3 mColor{};

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	};

	VkEngineModel(VkEngineDevice& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);

	~VkEngineModel();

	VkEngineModel(const VkEngineModel&) = delete;

	VkEngineModel& operator=(const VkEngineModel&) = delete;

	void bind(VkCommandBuffer commandBuffer) const;

	void draw(VkCommandBuffer commandBuffer) const;

   private:
	template <typename T, typename MemAlloc>
	void createVkBuffer(const std::vector<T>& data, VkBufferUsageFlags usage, VkBuffer& buffer,
	                    MemAlloc& bufferMemory) const;

	template <typename MemAlloc>
	void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
	                  MemAlloc& bufferMemory) const;

	void createVertexBuffers(const std::vector<Vertex>& vertices);

	void createIndexBuffers(const std::vector<uint32_t>& indices);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;

	template <typename MemAlloc>
	struct DataBuffer {
		VkBuffer pDataBuffer = VK_NULL_HANDLE;
		MemAlloc pDataBufferMemory = VK_NULL_HANDLE;
	};

	DataBuffer<VkDeviceMemory> mVertexBuffer{};
	DataBuffer<VkDeviceMemory> mIndexBuffer{};

	uint32_t mIndexCount = 0;

	VkEngineDevice& mDevice;
};
}  // namespace vke

#endif  // VKENGINEMODEL_HPP