//
// Created by zphrfx on 02/06/2024.
//

#pragma once

#include "engine_device.hpp"

namespace vke {

class VkEngineBuffer {
   public:
	VkEngineBuffer(VkEngineDevice& device, VkDeviceSize instanceSize, uint32_t instanceCount,
	               VkBufferUsageFlags usageFlags, VmaAllocationCreateFlags flag, VmaMemoryUsage memoryUsage, VkDeviceSize minOffsetAlignment = 1);
	~VkEngineBuffer();

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

	void* getMappedMemory() const { return pMapped; }
	uint32_t getInstanceCount() const { return mInstanceCount; }
	const VkBuffer& getBuffer() const { return pBuffer; }
	const VkDeviceSize& getInstanceSize() const { return mInstanceSize; }
	const VkDeviceSize& getAlignmentSize() const { return mAlignmentSize; }
	const VkBufferUsageFlags& getUsageFlags() const { return mUsageFlags; }
	const VmaMemoryUsage& getMemoryUsage() const { return mMemoryUsage; }
	const VkDeviceSize& getBufferSize() const { return mBufferSize; }
	const VmaAllocation& getBufferMemory() const { return pDataBufferMemory; }

   private:
	static VkDeviceSize getAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

	VkEngineDevice& mDevice;
	void* pMapped = nullptr;
	VkBuffer pBuffer = VK_NULL_HANDLE;
	VmaAllocation pDataBufferMemory = VK_NULL_HANDLE;

	VkDeviceSize mBufferSize{};
	uint32_t mInstanceCount{};
	VkDeviceSize mInstanceSize{};
	VkDeviceSize mAlignmentSize{};
	VkBufferUsageFlags mUsageFlags{};
	VmaMemoryUsage mMemoryUsage{};
};

}  // namespace vke