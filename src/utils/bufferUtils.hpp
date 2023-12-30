//
// Created by Enzo Crema on 28/12/2023.
//
#pragma once

namespace vke {
class BufferUtils {
   public:
	static void beginSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool,
	                                    VkCommandBuffer& commandBuffer);

	static void endSingleTimeCommands(const VkDevice& device, const VkCommandPool& commandPool,
	                                  VkCommandBuffer& commandBuffer, const VkQueue& graphicsQueue);


		[[maybe_unused]] static void copyBuffer(
		const VkDevice& device,
		const VkCommandPool& commandPool,
		VkCommandBuffer& commandBuffer,
		const VkBuffer& srcBuffer,
		const VkBuffer& dstBuffer,
		const VkDeviceSize& size);
};

}  // namespace vke