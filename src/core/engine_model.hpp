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
#include <utils/memory.hpp>

#include "engine_buffer.hpp"

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

		~MeshData() {
			if (!pVertices.empty()) {
				Memory::freeMemory(pVertices.data(), pVertices.size(), MEMORY_TAG_ENGINE);
			}
			if (!pIndices.empty()) {
				Memory::freeMemory(pIndices.data(), pIndices.size(), MEMORY_TAG_ENGINE);
			}
		}
	};

	VkEngineModel(std::shared_ptr<VkEngineDevice> device, const MeshData& meshData);

	~VkEngineModel();

	VkEngineModel(const VkEngineModel&) = delete;
	VkEngineModel& operator=(const VkEngineModel&) = delete;
	VkEngineModel(VkEngineModel&&) = default;  // Enable move semantics

	static std::unique_ptr<VkEngineModel> createModelFromFile(std::shared_ptr<VkEngineDevice> device,
	                                                          const std::string& filepath);
	void bind(const VkCommandBuffer* commandBuffer) const;
	void draw(const VkCommandBuffer* commandBuffer) const;

   private:
	template <typename T>
	void createVkBuffer(const std::span<const T>& data, VkBufferUsageFlags usageDst,
	                    std::unique_ptr<VkEngineBuffer>& buffer);


	void createVertexBuffers(const std::span<const Vertex>& vertices);

	void createIndexBuffers(const std::span<const u32>& indices);

	std::unique_ptr<VkEngineBuffer> mVertexBuffer{};
	std::unique_ptr<VkEngineBuffer> mIndexBuffer{};
	std::shared_ptr<VkEngineDevice> mDevice{};

	VkCommandBuffer mCommandBuffer = VK_NULL_HANDLE;
	size_t mIndexCount = 0;
};
}  // namespace vke