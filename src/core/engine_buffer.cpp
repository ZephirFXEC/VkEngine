#include "engine_buffer.hpp"

// std
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <utils/memory.hpp>


namespace vke {

VkDeviceSize VkEngineBuffer::getAlignment(const VkDeviceSize instanceSize, const VkDeviceSize minOffsetAlignment) {
	if (minOffsetAlignment > 0) {
		return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
	}
	return instanceSize;
}

VkEngineBuffer::VkEngineBuffer(VkEngineDevice &device, const VkDeviceSize instanceSize, const uint32_t instanceCount,
                               const VkBufferUsageFlags usageFlags, const VmaMemoryUsage memoryUsage,
                               const VkDeviceSize minOffsetAlignment)
    : lveDevice(device),
      instanceCount(instanceCount),
      instanceSize(instanceSize),
      alignmentSize(getAlignment(instanceSize, minOffsetAlignment)), usageFlags(usageFlags),
      memoryUsage(memoryUsage) {

	bufferSize = alignmentSize * instanceCount;

	// Create buffer with VMA
	const VkBufferCreateInfo bufferInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = bufferSize,
		.usage = usageFlags
	};

	const VmaAllocationCreateInfo allocInfo = {
		.usage = memoryUsage
	};

	if (vmaCreateBuffer(lveDevice.getAllocator(), &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) !=
	    VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer with VMA");
	}

}

VkEngineBuffer::~VkEngineBuffer() {
	if (buffer != VK_NULL_HANDLE) {
		vmaDestroyBuffer(lveDevice.getAllocator(), buffer, allocation);
	}
}

VkResult VkEngineBuffer::map(const VkDeviceSize size, const VkDeviceSize offset) {
	return vmaMapMemory(lveDevice.getAllocator(), allocation, &mapped);
}


void VkEngineBuffer::unmap() {
	if (mapped) {
		vmaUnmapMemory(lveDevice.getAllocator(), allocation);
		mapped = nullptr;
	}
}

void VkEngineBuffer::writeToBuffer(const void *data, const VkDeviceSize size, const VkDeviceSize offset) const {
	if(!mapped) {
		throw std::runtime_error("Cannot map memory to write");
	}

	const VkDeviceSize copySize = (size == VK_WHOLE_SIZE) ? bufferSize : size;

	if (offset + copySize > bufferSize) {
		throw std::out_of_range("Buffer write out of range");
	}

	Memory::copyMemory(static_cast<char *>(mapped) + offset, data, copySize);
}


VkResult VkEngineBuffer::flush(const VkDeviceSize size, const VkDeviceSize offset) const {
	return vmaFlushAllocation(lveDevice.getAllocator(), allocation, offset, size);
}

VkResult VkEngineBuffer::invalidate(const VkDeviceSize size, const VkDeviceSize offset) const {
	return vmaInvalidateAllocation(lveDevice.getAllocator(), allocation, offset, size);
}

VkDescriptorBufferInfo VkEngineBuffer::descriptorInfo(const VkDeviceSize size, const VkDeviceSize offset) const {
	return VkDescriptorBufferInfo{buffer, offset, size};
}


void VkEngineBuffer::writeToIndex(const void *data, const int index) const {
	writeToBuffer(data, instanceSize, index * alignmentSize);
}


VkResult VkEngineBuffer::flushIndex(const int index) const { return flush(alignmentSize, index * alignmentSize); }


VkDescriptorBufferInfo VkEngineBuffer::descriptorInfoForIndex(const int index) const {
	return descriptorInfo(alignmentSize, index * alignmentSize);
}


VkResult VkEngineBuffer::invalidateIndex(const int index) const {
	return invalidate(alignmentSize, index * alignmentSize);
}

}  // namespace vke
