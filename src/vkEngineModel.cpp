//
// Created by Enzo Crema on 21/12/2023.
//

#include "vkEngineModel.hpp"
#include <cassert>

namespace vke
{
VkEngineModel::VkEngineModel(VkEngineDevice& device, const std::vector <Vertex>& vertices)
	: mDevice{device}
{
	createVertexBuffers(vertices);
}

VkEngineModel::~VkEngineModel()
{
	vkDestroyBuffer(mDevice.device(), pVertexBuffer, nullptr);
	vkFreeMemory(mDevice.device(), pVertexBufferMemory, nullptr);
}

std::vector <VkVertexInputBindingDescription> VkEngineModel::Vertex::getBindingDescriptions()
{
	std::vector <VkVertexInputBindingDescription> bindingDescriptions{
		{0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX}};

	return bindingDescriptions;

}

std::vector <VkVertexInputAttributeDescription> VkEngineModel::Vertex::getAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{
		{0, 0, VK_FORMAT_R32G32_SFLOAT, 0}};

	return attributeDescriptions;
}

void VkEngineModel::bind(const VkCommandBuffer commandBuffer) const
{
	const VkBuffer buffers[] = {pVertexBuffer};
	constexpr VkDeviceSize offsets[] = {0};

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void VkEngineModel::draw(const VkCommandBuffer commandBuffer) const
{
	vkCmdDraw(commandBuffer, mVertexCount, 1, 0, 0);
}

void VkEngineModel::createVertexBuffers(const std::vector <Vertex>& vertices)
{
	mVertexCount = static_cast <uint32_t>(vertices.size());
	assert(mVertexCount >= 3 && "Vertex count must be at least 3");

	const VkDeviceSize bufferSize = sizeof(vertices[0]) * mVertexCount;

	mDevice.createBuffer(bufferSize,
	                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
	                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	                     pVertexBuffer,
	                     pVertexBufferMemory);

	void* data;
	vkMapMemory(mDevice.device(), pVertexBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast <size_t>(bufferSize));
	vkUnmapMemory(mDevice.device(), pVertexBufferMemory);
}
} // vke