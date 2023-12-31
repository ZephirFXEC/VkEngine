//
// Created by Enzo Crema on 28/12/2023.
//

#include "bufferUtils.hpp"

namespace vke {

void BufferUtils::beginSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool,
                                          VkCommandBuffer& commandBuffer) {
	const VkCommandBufferAllocateInfo allocInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	    .commandPool = commandPool,
	    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	    .commandBufferCount = 1,
	};

	VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

	constexpr VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	                                             .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

	VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
}


void BufferUtils::endSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool,
                                        VkCommandBuffer& commandBuffer, const VkQueue& graphicsQueue) {
	VK_CHECK(vkEndCommandBuffer(commandBuffer));

	const VkSubmitInfo submitInfo{
	    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &commandBuffer};

	VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(graphicsQueue));

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}


void BufferUtils::createModelBuffer(const VkEngineDevice& device, const VkDeviceSize size,
                                    const VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& bufferMemory, const VmaMemoryUsage memoryUsage) {

	const VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                                    .size = size,
	                                    .usage = usage,
	                                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

	const VmaAllocationCreateInfo allocInfo{
	    .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT,
	    .usage = memoryUsage,
	};

	VK_CHECK(vmaCreateBuffer(device.getAllocator(), &bufferInfo, &allocInfo, &buffer, &bufferMemory, nullptr));
}
}  // namespace vke