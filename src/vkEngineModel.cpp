//
// Created by Enzo Crema on 21/12/2023.
//

#include "vkEngineModel.hpp"
#include <cassert>

namespace vke
{
VkEngineModel::VkEngineModel(VkEngineDevice& device, const std::vector<Vertex>& vertices)
	: mDevice{device}
{
	createVertexBuffers(vertices);
}

VkEngineModel::~VkEngineModel()
{
	vkDestroyBuffer(mDevice.device(), pVertexBuffer, nullptr);
	vkFreeMemory(mDevice.device(), pVertexBufferMemory, nullptr);
}

std::vector<VkVertexInputBindingDescription> VkEngineModel::Vertex::getBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions{
		{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};

	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> VkEngineModel::Vertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{
		{0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, mPosition)},
		{1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mColor)}};

	return attributeDescriptions;
}

void VkEngineModel::bind(const VkCommandBuffer commandBuffer) const
{
	const std::array<VkBuffer, 1> buffers{pVertexBuffer};
	constexpr std::array<VkDeviceSize, 1> offsets{0};

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers.data(), offsets.data());
}

void VkEngineModel::draw(const VkCommandBuffer commandBuffer) const
{
	vkCmdDraw(commandBuffer, mVertexCount, 1, 0, 0);
}

void VkEngineModel::createVertexBuffers(const std::vector<Vertex>& vertices)
{
	mVertexCount = static_cast<uint32_t>(vertices.size());
	assert(mVertexCount >= 3 && "Vertex count must be at least 3");

	const VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = sizeof(vertices[0]) * vertices.size(),
		.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
	};

	if(vkCreateBuffer(mDevice.device(), &bufferInfo, nullptr, &pVertexBuffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create vertex buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(mDevice.device(), pVertexBuffer, &memRequirements);

	const VkMemoryAllocateInfo allocInfo
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memRequirements.size,
		.memoryTypeIndex = mDevice.findMemoryType(memRequirements.memoryTypeBits,
												  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
													  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT),
	};

	if(vkAllocateMemory(mDevice.device(), &allocInfo, nullptr, &pVertexBufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate vertex buffer memory!");
	}

	vkBindBufferMemory(mDevice.device(), pVertexBuffer, pVertexBufferMemory, 0);

	void* data = nullptr;
	vkMapMemory(mDevice.device(), pVertexBufferMemory, 0, bufferInfo.size, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferInfo.size);
	vkUnmapMemory(mDevice.device(), pVertexBufferMemory);
}
} // namespace vke