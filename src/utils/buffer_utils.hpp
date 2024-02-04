//
// Created by Enzo Crema on 28/12/2023.
//
#pragma once

#include <vk_mem_alloc.h>

#include "core/engine_device.hpp"
#include "logger.hpp"


namespace vke {
class BufferUtils {
   public:
	static void beginSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool,
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


	static void endSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool,
	                                  VkCommandBuffer& commandBuffer, const VkQueue& graphicsQueue) {
		VK_CHECK(vkEndCommandBuffer(commandBuffer));

		const VkSubmitInfo submitInfo{
		    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO, .commandBufferCount = 1, .pCommandBuffers = &commandBuffer};

		VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));
		VK_CHECK(vkQueueWaitIdle(graphicsQueue));

		vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
	}
};
}  // namespace vke
