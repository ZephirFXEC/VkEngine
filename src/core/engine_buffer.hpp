//
// Created by zphrfx on 02/06/2024.
//

#pragma once

#include "engine_device.hpp"

namespace vke {

class VkEngineBuffer {
   public:
	VkEngineBuffer(VkEngineDevice& device, VkDeviceSize instanceSize, uint32_t instanceCount,
	               VkBufferUsageFlags usageFlags, VmaMemoryUsage memoryUsage, VkDeviceSize minOffsetAlignment = 1);
	~VkEngineBuffer();

	VkEngineBuffer(const VkEngineBuffer&) = delete;
	VkEngineBuffer& operator=(const VkEngineBuffer&) = delete;

	VkResult map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	void unmap();

	void writeToBuffer(const void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
	VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
	VkDescriptorBufferInfo descriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;
	VkResult invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0) const;

	void writeToIndex(const void* data, int index) const;
	VkResult flushIndex(int index) const;
	VkDescriptorBufferInfo descriptorInfoForIndex(int index) const;
	VkResult invalidateIndex(int index) const;

	void* getMappedMemory() const { return mapped; }
	uint32_t getInstanceCount() const { return instanceCount; }
	const VkBuffer& getBuffer() const { return buffer; }
	const VkDeviceSize& getInstanceSize() const { return instanceSize; }
	const VkDeviceSize& getAlignmentSize() const { return alignmentSize; }
	const VkBufferUsageFlags& getUsageFlags() const { return usageFlags; }
	const VmaMemoryUsage& getMemoryUsage() const { return memoryUsage; }
	const VkDeviceSize& getBufferSize() const { return bufferSize; }

   private:
	static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

	VkEngineDevice& lveDevice;
	void* mapped = nullptr;
	VkBuffer buffer = VK_NULL_HANDLE;
	VmaAllocation allocation = VK_NULL_HANDLE;

	VkDeviceSize bufferSize{};
	uint32_t instanceCount{};
	VkDeviceSize instanceSize{};
	VkDeviceSize alignmentSize{};
	VkBufferUsageFlags usageFlags{};
	VmaMemoryUsage memoryUsage{};
};

}  // namespace vke