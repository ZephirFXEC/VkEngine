//
// Created by Enzo Crema on 21/12/2023.
//

#ifndef VKENGINEMODEL_HPP
#define VKENGINEMODEL_HPP

// engine
#include "vkEngineDevice.hpp"

// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

// std
#include <vector>

namespace vke {

class VkEngineModel {
public:

	struct Vertex
	{
		glm::vec2 mPosition{};
		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
	};

	VkEngineModel(VkEngineDevice &device, const std::vector<Vertex> &vertices);
	~VkEngineModel();

	VkEngineModel(const VkEngineModel &) = delete;
	VkEngineModel &operator=(const VkEngineModel &) = delete;

	void bind(VkCommandBuffer commandBuffer) const;
	void draw(VkCommandBuffer commandBuffer) const;

private:

	void createVertexBuffers(const std::vector<Vertex> &vertices);

	VkEngineDevice& mDevice;
	VkBuffer pVertexBuffer = VK_NULL_HANDLE;
	VkDeviceMemory pVertexBufferMemory = VK_NULL_HANDLE;
	uint32_t mVertexCount = 0;

};

} // vke

#endif //VKENGINEMODEL_HPP