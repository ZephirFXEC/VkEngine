//
// Created by Enzo Crema on 28/12/2023.
//
#pragma once

#include "utility.hpp"

namespace vke {
class BufferUtils {
   public:
	static void beginSingleTimeCommands(const VkDevice& device, FrameData& frameData);

	static void endSingleTimeCommands(const VkDevice& device, FrameData& frameData, const VkQueue& graphicsQueue);

	static void copyBuffer(const VkDevice& device, FrameData& frameData, const VkBuffer& srcBuffer, const VkBuffer& dstBuffer,
	                       const VkDeviceSize& size);
};

}  // namespace vke