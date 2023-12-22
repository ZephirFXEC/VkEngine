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

	const VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer = nullptr;
	VkDeviceMemory stagingBufferMemory = nullptr;
	createBuffer(bufferSize,
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				 stagingBuffer,
				 stagingBufferMemory);

	void* data = nullptr;
	vkMapMemory(mDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(mDevice.device(), stagingBufferMemory);

	createBuffer(bufferSize,
				 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				 pVertexBuffer,
				 pVertexBufferMemory);

	copyBuffer(stagingBuffer, pVertexBuffer, bufferSize);

	vkDestroyBuffer(mDevice.device(), stagingBuffer, nullptr);
	vkFreeMemory(mDevice.device(), stagingBufferMemory, nullptr);
}
void VkEngineModel::createBuffer(const VkDeviceSize size,
								 const VkBufferUsageFlags usage,
								 const VkMemoryPropertyFlags properties,
								 VkBuffer& buffer,
								 VkDeviceMemory& bufferMemory) const
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if(vkCreateBuffer(mDevice.device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(mDevice.device(), buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = mDevice.findMemoryType(memRequirements.memoryTypeBits, properties);

	if(vkAllocateMemory(mDevice.device(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(mDevice.device(), buffer, bufferMemory, 0);
}
void VkEngineModel::copyBuffer(const VkBuffer srcBuffer,
							   const VkBuffer dstBuffer,
							   const VkDeviceSize size) const
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = mDevice.getCommandPool();
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = nullptr;
	vkAllocateCommandBuffers(mDevice.device(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion{};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(mDevice.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mDevice.graphicsQueue());

	vkFreeCommandBuffers(mDevice.device(), mDevice.getCommandPool(), 1, &commandBuffer);
}
} // namespace vke