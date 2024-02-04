//
// Created by Enzo Crema on 21/12/2023.
//

#pragma once

// engine
#include "engine_device.hpp"
#include "engine_swapchain.hpp"

// glm
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vke {
class VkEngineModel {
   public:
	struct Vertex {
		glm::vec3 mPosition{};
		glm::vec3 mColor{};

		static std::array<VkVertexInputBindingDescription, 1> getBindingDescriptions();

		static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions();
	};

	struct MeshData {
		Vertex* pVertices;
		u32 vCount;
		u32* pIndices;
		u32 iCount;
	};

	VkEngineModel(const VkEngineDevice& device, std::shared_ptr<VkEngineSwapChain> swapchain, const MeshData& meshData);

	~VkEngineModel();

	VkEngineModel(const VkEngineModel&) = delete;
	VkEngineModel& operator=(const VkEngineModel&) = delete;
	VkEngineModel(VkEngineModel&&) = default;  // Enable move semantics

	void bind(const VkCommandBuffer* commandBuffer) const;
	void draw(const VkCommandBuffer* commandBuffer) const;

   private:
	template <typename T>
	void createVkBuffer(const T* data, size_t dataSize, VkBufferUsageFlags usage, VkBuffer& buffer,
	                    VmaAllocation& bufferMemory);


	void createVertexBuffers(const Vertex* vertices, size_t vertexCount);

	void createIndexBuffers(const u32* indices, size_t indexCount);

	DataBuffer mVertexBuffer{};
	DataBuffer mIndexBuffer{};

	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

	u32 mIndexCount = 0;

	const VkEngineDevice& mDevice;

	// note don't access mDevice using the swap chain, since it's a shared pointer it will be nullptr when resizing
	// the window
	const std::shared_ptr<VkEngineSwapChain> pSwapChain;
};
}  // namespace vke