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

#include <span>

namespace vke {
class VkEngineModel {
   public:
	struct Vertex {
		glm::vec3 mPosition{};
		glm::vec3 mColor{};
		glm::vec3 mNormal{};
		glm::vec2 mUV{};

		bool operator==(const Vertex& other) const {
			return mPosition == other.mPosition && mColor == other.mColor && mNormal == other.mNormal &&
			       mUV == other.mUV;
		}
	};

	static std::array<VkVertexInputBindingDescription, 1> getBindingDescriptions();
	static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();


	struct MeshData {
		std::span<const Vertex> pVertices;
		std::span<const u32> pIndices;
		void loadModel(const std::string& filepath);
	};

	VkEngineModel(const VkEngineDevice& device, const MeshData& meshData);

	~VkEngineModel();

	VkEngineModel(const VkEngineModel&) = delete;
	VkEngineModel& operator=(const VkEngineModel&) = delete;
	VkEngineModel(VkEngineModel&&) = default;  // Enable move semantics

	static std::unique_ptr<VkEngineModel> createModelFromFile(const VkEngineDevice& device,
	                                                          const std::string& filepath);
	void bind(const VkCommandBuffer* commandBuffer) const;
	void draw(const VkCommandBuffer* commandBuffer) const;

   private:
	template <typename T>
	void createVkBuffer(const T* data, size_t dataSize, VkBufferUsageFlags usage, VkBuffer& buffer,
	                    VmaAllocation& bufferMemory);


	void createVertexBuffers(const std::span<const Vertex>& vertices);

	void createIndexBuffers(const std::span<const u32>& indices);

	DataBuffer mVertexBuffer{};
	DataBuffer mIndexBuffer{};

	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

	size_t mIndexCount = 0;

	const VkEngineDevice& mDevice;

	// note don't access mDevice using the swap chain, since it's a shared pointer it will be nullptr when resizing
	// the window
};
}  // namespace vke