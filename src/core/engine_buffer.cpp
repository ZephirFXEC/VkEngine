#include "engine_buffer.hpp"

// std
#include <stdexcept>
#include <utils/memory.hpp>


namespace vke {

VkDeviceSize VkEngineBuffer::getAlignment(const VkDeviceSize instanceSize, const VkDeviceSize minOffsetAlignment) {
	if (minOffsetAlignment > 0) {
		return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
	}
	return instanceSize;
}

VkEngineBuffer::VkEngineBuffer(std::shared_ptr<VkEngineDevice> device, const VkDeviceSize instanceSize, const uint32_t instanceCount,
                               const VkBufferUsageFlags usageFlags, const VmaAllocationCreateFlags flag, const VmaMemoryUsage memoryUsage,
                               const VkDeviceSize minOffsetAlignment)
    : mDevice(std::move(device)),
      mInstanceCount(instanceCount),
      mInstanceSize(instanceSize),
      mAlignmentSize(getAlignment(instanceSize, minOffsetAlignment)),
      mUsageFlags(usageFlags),
      mMemoryUsage(memoryUsage) {
	mBufferSize = mAlignmentSize * instanceCount;

	// Create buffer with VMA
	const VkBufferCreateInfo bufferInfo = {
	    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	    .size = mBufferSize,
	    .usage = usageFlags,
	};

	const VmaAllocationCreateInfo allocCreateInfo = {
		.flags = flag,
		.usage = memoryUsage,
	};

	if (vmaCreateBuffer(mDevice->getAllocator(), &bufferInfo, &allocCreateInfo, &pBuffer, &pDataBufferMemory, nullptr) !=
	    VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer with VMA");
	}
}

VkEngineBuffer::~VkEngineBuffer() {
	if (pBuffer != VK_NULL_HANDLE) {
		vmaDestroyBuffer(mDevice->getAllocator(), pBuffer, pDataBufferMemory);
	}
}

VkResult VkEngineBuffer::map(const VkDeviceSize size, const VkDeviceSize offset) {
	return vmaMapMemory(mDevice->getAllocator(), pDataBufferMemory, &pMapped);
}


void VkEngineBuffer::unmap() {
	if (pMapped) {
		vmaUnmapMemory(mDevice->getAllocator(), pDataBufferMemory);
		pMapped = nullptr;
	}
}

void VkEngineBuffer::writeToBuffer(const void *data, const VkDeviceSize size, const VkDeviceSize offset) const {
	if (!pMapped) {
		throw std::runtime_error("Cannot map memory to write");
	}

	const VkDeviceSize copySize = (size == VK_WHOLE_SIZE) ? mBufferSize : size;

	if (offset + copySize > mBufferSize) {
		throw std::out_of_range("Buffer write out of range");
	}

	Memory::copyMemory(static_cast<char *>(pMapped) + offset, data, copySize);
}


VkResult VkEngineBuffer::flush(const VkDeviceSize size, const VkDeviceSize offset) const {
	return vmaFlushAllocation(mDevice->getAllocator(), pDataBufferMemory, offset, size);
}

VkResult VkEngineBuffer::invalidate(const VkDeviceSize size, const VkDeviceSize offset) const {
	return vmaInvalidateAllocation(mDevice->getAllocator(), pDataBufferMemory, offset, size);
}

VkDescriptorBufferInfo VkEngineBuffer::descriptorInfo(const VkDeviceSize size, const VkDeviceSize offset) const {
	return VkDescriptorBufferInfo{pBuffer, offset, size};
}


void VkEngineBuffer::writeToIndex(const void *data, const int index) const {
	writeToBuffer(data, mInstanceSize, index * mAlignmentSize);
}


VkResult VkEngineBuffer::flushIndex(const int index) const { return flush(mAlignmentSize, index * mAlignmentSize); }


VkDescriptorBufferInfo VkEngineBuffer::descriptorInfoForIndex(const int index) const {
	return descriptorInfo(mAlignmentSize, index * mAlignmentSize);
}


VkResult VkEngineBuffer::invalidateIndex(const int index) const {
	return invalidate(mAlignmentSize, index * mAlignmentSize);
}

}  // namespace vke
