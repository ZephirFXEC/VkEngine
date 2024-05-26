//
// Created by Enzo Crema on 21/12/2023.
//

#pragma once

// engine
#include "engine_device.hpp"
#include "engine_swapchain.hpp"
#include "utils/hash.hpp"

// glm
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


namespace vke {
class VkEngineModel {
   public:
	struct Vertex {
		glm::vec3 mPosition{};
		glm::vec3 mColor{};
		glm::vec3 mNormal{};
		glm::vec2 mUV{};

		static std::array<VkVertexInputBindingDescription, 1> getBindingDescriptions();
		static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();

		bool operator==(const Vertex& other) const {
			return mPosition == other.mPosition && mColor == other.mColor && mNormal == other.mNormal &&
			       mUV == other.mUV;
		}
	};

	struct MeshData {
		Vertex* pVertices;
		u32 vCount;
		u32* pIndices;
		u32 iCount;

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


	void createVertexBuffers(const Vertex* vertices, size_t vertexCount);

	void createIndexBuffers(const u32* indices, size_t indexCount);

	DataBuffer mVertexBuffer{};
	DataBuffer mIndexBuffer{};

	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;

	u32 mIndexCount = 0;

	const VkEngineDevice& mDevice;

	// note don't access mDevice using the swap chain, since it's a shared pointer it will be nullptr when resizing
	// the window
};
}  // namespace vke