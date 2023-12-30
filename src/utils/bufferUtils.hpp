//
// Created by Enzo Crema on 28/12/2023.
//
#pragma once

#include "utility.hpp"
#include "vkEngineDevice.hpp"

namespace vke {
class BufferUtils {
   public:
	static void beginSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool,
	                                    VkCommandBuffer& commandBuffer);

	static void endSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool,
	                                  VkCommandBuffer& commandBuffer, const VkQueue& graphicsQueue);

	static void createModelBuffer(const VkEngineDevice& device, VkDeviceSize size, VkBufferUsageFlags usage,
	                              VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	static void createModelBuffer(const VkEngineDevice& device, VkDeviceSize size, VkBufferUsageFlags usage,
	                              VkBuffer& buffer, VmaAllocation& bufferMemory);
};

}  // namespace vke