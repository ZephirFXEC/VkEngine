//
// Created by Enzo Crema on 21/12/2023.
//

#include "vkEngineModel.hpp"
#include <cassert>

namespace vke {
VkEngineModel::VkEngineModel(VkEngineDevice& device, const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices):
	mIndexCount{ static_cast<uint32_t>(indices.size()) }, mDevice{ device }
{
	createIndexBuffers(indices);
	createVertexBuffers(vertices);
}

VkEngineModel::~VkEngineModel()
{
	vkDestroyBuffer(mDevice.device(), mVertexBuffer.pVertexBuffer, nullptr);
	vkFreeMemory(mDevice.device(), mVertexBuffer.pVertexBufferMemory, nullptr);

	vkDestroyBuffer(mDevice.device(), mIndexBuffer.pIndexBuffer, nullptr);
	vkFreeMemory(mDevice.device(), mIndexBuffer.pIndexBufferMemory, nullptr);
}

std::vector<VkVertexInputBindingDescription> VkEngineModel::Vertex::getBindingDescriptions()
{
	return { { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX } };
}

std::vector<VkVertexInputAttributeDescription> VkEngineModel::Vertex::getAttributeDescriptions()
{
	return { { 0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, mPosition) },
			 { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, mColor) } };
}

void VkEngineModel::bind(const VkCommandBuffer commandBuffer) const
{
	const std::array                      buffers{ mVertexBuffer.pVertexBuffer };
	constexpr std::array<VkDeviceSize, 1> offsets{ 0 };

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers.data(), offsets.data());
	vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer.pIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void VkEngineModel::draw(const VkCommandBuffer commandBuffer) const { vkCmdDrawIndexed(commandBuffer, mIndexCount, 1, 0, 0, 0); }


template <typename T>
void VkEngineModel::createVkBuffer(
	const std::vector<T>& data, const VkBufferUsageFlags usage, VkBuffer& buffer, VkDeviceMemory& bufferMemory) const
{
	const VkDeviceSize bufferSize = sizeof(data[0]) * data.size();

	VkBuffer       stagingBuffer = nullptr;
	VkDeviceMemory stagingBufferMemory = nullptr;
	createBuffer(
		bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer, stagingBufferMemory);

	void* stagingBufferData = nullptr;
	vkMapMemory(mDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &stagingBufferData);
	memcpy(stagingBufferData, data.data(), static_cast<size_t>(bufferSize));
	vkUnmapMemory(mDevice.device(), stagingBufferMemory);

	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, buffer, bufferMemory);

	copyBuffer(stagingBuffer, buffer, bufferSize);

	vkDestroyBuffer(mDevice.device(), stagingBuffer, nullptr);
	vkFreeMemory(mDevice.device(), stagingBufferMemory, nullptr);
}


void VkEngineModel::createVertexBuffers(const std::vector<Vertex>& vertices)
{
	createVkBuffer(vertices, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mVertexBuffer.pVertexBuffer, mVertexBuffer.pVertexBufferMemory);
}

void VkEngineModel::createIndexBuffers(const std::vector<uint32_t>& indices)
{
	createVkBuffer(indices, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mIndexBuffer.pIndexBuffer, mIndexBuffer.pIndexBufferMemory);
}

void VkEngineModel::createBuffer(
	const VkDeviceSize size, const VkBufferUsageFlags usage, const VkMemoryPropertyFlags properties, VkBuffer& buffer,
	VkDeviceMemory& bufferMemory) const
{
	const VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size = size, .usage = usage, .sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	if (vkCreateBuffer(mDevice.device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(mDevice.device(), buffer, &memRequirements);

	const VkMemoryAllocateInfo allocInfo{ .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
										  .allocationSize = memRequirements.size,
										  .memoryTypeIndex = mDevice.findMemoryType(memRequirements.memoryTypeBits, properties) };

	if (vkAllocateMemory(mDevice.device(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(mDevice.device(), buffer, bufferMemory, 0);
}
void VkEngineModel::copyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size) const
{
	const VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandPool = mDevice.getCommandPool(),
		.commandBufferCount = 1,
	};

	VkCommandBuffer commandBuffer = nullptr;
	vkAllocateCommandBuffers(mDevice.device(), &allocInfo, &commandBuffer);

	constexpr VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
												  .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT };

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	const VkBufferCopy copyRegion{ .size = size };
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	const VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
	};

	vkQueueSubmit(mDevice.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mDevice.graphicsQueue());

	vkFreeCommandBuffers(mDevice.device(), mDevice.getCommandPool(), 1, &commandBuffer);
}
}    // namespace vke