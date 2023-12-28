//
// Created by Enzo Crema on 28/12/2023.
//

#include "bufferUtils.hpp"

namespace vke {

void BufferUtils::beginSingleTimeCommands(const VkDevice& device, FrameData& frameData) {
	const VkCommandBufferAllocateInfo allocInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	    .commandPool = frameData.pCommandPool,
	    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	    .commandBufferCount = 1,
	};

	VK_CHECK(vkAllocateCommandBuffers(device, &allocInfo, &frameData.pCommandBuffer));

	constexpr VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	                                             .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

	VK_CHECK(vkBeginCommandBuffer(frameData.pCommandBuffer, &beginInfo));
}

void BufferUtils::endSingleTimeCommands(const VkDevice& device, FrameData& frameData, const VkQueue& graphicsQueue) {
	VK_CHECK(vkEndCommandBuffer(frameData.pCommandBuffer));

	const VkSubmitInfo submitInfo{
	    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &frameData.pCommandBuffer};

	VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK(vkQueueWaitIdle(graphicsQueue));

	vkFreeCommandBuffers(device, frameData.pCommandPool, 1, &frameData.pCommandBuffer);
}
}  // namespace vke