//
// Created by Enzo Crema on 21/12/2023.
//

#include "vkEngineModel.hpp"

namespace vke {
VkEngineModel::VkEngineModel(VkEngineDevice& device, const Vertex* vertices, const uint32_t vCount,
                             const uint32_t* indices, const uint32_t iCount)

	: mIndexCount{iCount}, mDevice{device} {
	createIndexBuffers(indices, iCount);
	createVertexBuffers(vertices, vCount);
}

VkEngineModel::~VkEngineModel() {
	destroyBuffer(mVertexBuffer);
	destroyBuffer(mIndexBuffer);
}

template <typename MemAlloc>
void VkEngineModel::destroyBuffer(const DataBuffer<MemAlloc>& buffer) const {
	if constexpr (!USE_VMA) {
		vkDestroyBuffer(mDevice.device(), buffer.pDataBuffer, nullptr);
		vkFreeMemory(mDevice.device(), buffer.pDataBufferMemory, nullptr);
	} else {
		vmaDestroyBuffer(mDevice.getAllocator(), buffer.pDataBuffer, buffer.pDataBufferMemory);
	}
}

VkVertexInputBindingDescription* VkEngineModel::Vertex::getBindingDescriptions() {
	return new VkVertexInputBindingDescription[1]{
		{.binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}};
}

VkVertexInputAttributeDescription* VkEngineModel::Vertex::getAttributeDescriptions() {
	return new VkVertexInputAttributeDescription[2]{
		{.binding = 0, .location = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, mPosition)},
		{.binding = 0, .location = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, mColor)}};
}

void VkEngineModel::bind(const VkCommandBuffer commandBuffer) const {
	const std::array buffers{mVertexBuffer.pDataBuffer};
	constexpr std::array<VkDeviceSize, 1> offsets{0};

	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers.data(), offsets.data());
	vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer.pDataBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void VkEngineModel::draw(const VkCommandBuffer commandBuffer) const {
	vkCmdDrawIndexed(commandBuffer, mIndexCount, 1, 0, 0, 0);
}

template <typename T, typename MemAlloc>
void VkEngineModel::createVkBuffer(const T* data, const size_t dataSize, const VkBufferUsageFlags usage,
                                   VkBuffer& buffer, MemAlloc& bufferMemory) const {
	const VkDeviceSize bufferSize = sizeof(T) * dataSize;

	VkBuffer stagingBuffer = nullptr;
	MemAlloc stagingBufferMemory = nullptr;

	createBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
	             stagingBufferMemory);

	void* mappedData = nullptr;

	if constexpr (!USE_VMA) {
		vkMapMemory(mDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &mappedData);
	} else {
		vmaMapMemory(mDevice.getAllocator(), stagingBufferMemory, &mappedData);
	}

	memcpy(mappedData, data, static_cast<size_t>(bufferSize));

	if constexpr (!USE_VMA) {
		vkUnmapMemory(mDevice.device(), stagingBufferMemory);
	} else {
		vmaUnmapMemory(mDevice.getAllocator(), stagingBufferMemory);
	}

	createBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);

	copyBuffer(stagingBuffer, buffer, bufferSize);

	if constexpr (!USE_VMA) {
		vkDestroyBuffer(mDevice.device(), stagingBuffer, nullptr);
		vkFreeMemory(mDevice.device(), stagingBufferMemory, nullptr);
	} else {
		vmaDestroyBuffer(mDevice.getAllocator(), stagingBuffer, stagingBufferMemory);
	}
}

void VkEngineModel::createVertexBuffers(const Vertex* vertices, const size_t vertexCount) {
	createVkBuffer(vertices, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mVertexBuffer.pDataBuffer,
	               mVertexBuffer.pDataBufferMemory);
}

void VkEngineModel::createIndexBuffers(const uint32_t* indices, const size_t indexCount) {
	createVkBuffer(indices, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mIndexBuffer.pDataBuffer,
	               mIndexBuffer.pDataBufferMemory);
}

template <typename MemAlloc>
void VkEngineModel::createBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage,
                                 const VkMemoryPropertyFlags properties, VkBuffer& buffer,
                                 MemAlloc& bufferMemory) const {
	const VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                                    .size = size,
	                                    .usage = usage,
	                                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

	if constexpr (!USE_VMA) {
		if (vkCreateBuffer(mDevice.device(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to create buffer!");
		}

		VkMemoryRequirements memRequirements{};
		vkGetBufferMemoryRequirements(mDevice.device(), buffer, &memRequirements);

		const VkMemoryAllocateInfo allocInfo{
			.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = memRequirements.size,
			.memoryTypeIndex = mDevice.findMemoryType(memRequirements.memoryTypeBits, properties)};

		if (vkAllocateMemory(mDevice.device(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate buffer memory!");
		}

		vkBindBufferMemory(mDevice.device(), buffer, bufferMemory, 0);
	} else {
		constexpr VmaAllocationCreateInfo allocInfo{
			.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
			.usage = VMA_MEMORY_USAGE_AUTO,
		};

		vmaCreateBuffer(mDevice.getAllocator(), &bufferInfo, &allocInfo, &buffer, &bufferMemory, nullptr);
	}
}

void VkEngineModel::copyBuffer(const VkBuffer srcBuffer, const VkBuffer dstBuffer, const VkDeviceSize size) const {
	const VkCommandBufferAllocateInfo allocInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = mDevice.getCommandPool(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer commandBuffer = nullptr;
	vkAllocateCommandBuffers(mDevice.device(), &allocInfo, &commandBuffer);

	constexpr VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	                                             .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	const VkBufferCopy copyRegion{.size = size};
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(commandBuffer);

	const VkSubmitInfo submitInfo{
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
	};

	vkQueueSubmit(mDevice.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mDevice.graphicsQueue());

	vkFreeCommandBuffers(mDevice.device(), mDevice.getCommandPool(), 1, &commandBuffer);
}
} // namespace vke
