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

void VkEngineModel::destroyBuffer(const DataBuffer& buffer) const {
#ifdef USE_VMA
	vmaDestroyBuffer(mDevice.getAllocator(), buffer.pDataBuffer, buffer.pDataBufferMemory);
#else
	vkDestroyBuffer(mDevice.device(), buffer.pDataBuffer, nullptr);
	vkFreeMemory(mDevice.device(), buffer.pDataBufferMemory, nullptr);
#endif
}

std::unique_ptr<std::array<VkVertexInputBindingDescription, 1>> VkEngineModel::Vertex::getBindingDescriptions() {
	return std::make_unique<std::array<VkVertexInputBindingDescription, 1>>(std::array{VkVertexInputBindingDescription{
	    .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}});
}

std::unique_ptr<std::array<VkVertexInputAttributeDescription, 2>> VkEngineModel::Vertex::getAttributeDescriptions() {
	return std::make_unique<std::array<VkVertexInputAttributeDescription, 2>>(std::array{
	    VkVertexInputAttributeDescription{
	        .binding = 0, .location = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, mPosition)},
	    VkVertexInputAttributeDescription{
	        .binding = 0, .location = 1, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, mColor)}});
}

void VkEngineModel::bind(const VkCommandBuffer* const commandBuffer) const {
	const std::array buffers{mVertexBuffer.pDataBuffer};
	constexpr std::array<VkDeviceSize, 1> offsets{0};

	vkCmdBindVertexBuffers(*commandBuffer, 0, 1, buffers.data(), offsets.data());
	vkCmdBindIndexBuffer(*commandBuffer, mIndexBuffer.pDataBuffer, 0, VK_INDEX_TYPE_UINT32);
}

void VkEngineModel::draw(const VkCommandBuffer* const commandBuffer) const {
	vkCmdDrawIndexed(*commandBuffer, mIndexCount, 1, 0, 0, 0);
}

template <typename T>
void VkEngineModel::createVkBuffer(const T* data, const size_t dataSize, const VkBufferUsageFlags usage,
                                   VkBuffer& buffer, Alloc& bufferMemory) {
	const VkDeviceSize bufferSize = sizeof(T) * dataSize;

	VkBuffer stagingBuffer = nullptr;
	Alloc stagingBufferMemory = nullptr;

	createBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
	             stagingBufferMemory);

	void* mappedData = nullptr;

#ifdef USE_VMA
	vmaMapMemory(mDevice.getAllocator(), stagingBufferMemory, &mappedData);
#else
	vkMapMemory(mDevice.device(), stagingBufferMemory, 0, bufferSize, 0, &mappedData);
#endif

	memcpy(mappedData, data, static_cast<size_t>(bufferSize));

#ifdef USE_VMA
	vmaUnmapMemory(mDevice.getAllocator(), stagingBufferMemory);
#else
	vkUnmapMemory(mDevice.device(), stagingBufferMemory);
#endif

	createBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);

	copyBuffer(&stagingBuffer, &buffer, bufferSize);

#ifdef USE_VMA
	vmaDestroyBuffer(mDevice.getAllocator(), stagingBuffer, stagingBufferMemory);
#else
	vkDestroyBuffer(mDevice.device(), stagingBuffer, nullptr);
	vkFreeMemory(mDevice.device(), stagingBufferMemory, nullptr);
#endif
}

void VkEngineModel::createVertexBuffers(const Vertex* vertices, const size_t vertexCount) {
	createVkBuffer(vertices, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mVertexBuffer.pDataBuffer,
	               mVertexBuffer.pDataBufferMemory);
}

void VkEngineModel::createIndexBuffers(const uint32_t* indices, const size_t indexCount) {
	createVkBuffer(indices, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mIndexBuffer.pDataBuffer,
	               mIndexBuffer.pDataBufferMemory);
}

void VkEngineModel::createBuffer(const VkDeviceSize size, const VkBufferUsageFlags usage,
                                 const VkMemoryPropertyFlags properties, VkBuffer& buffer, Alloc& bufferMemory) const {
	const VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                                    .size = size,
	                                    .usage = usage,
	                                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

#ifdef USE_VMA
	constexpr VmaAllocationCreateInfo allocInfo{
	    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
	    .usage = VMA_MEMORY_USAGE_AUTO,
	};

	if (vmaCreateBuffer(mDevice.getAllocator(), &bufferInfo, &allocInfo, &buffer, &bufferMemory, nullptr) !=
	    VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}
#else
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
#endif
}

void VkEngineModel::copyBuffer(const VkBuffer* const srcBuffer, const VkBuffer* const dstBuffer,
                               const VkDeviceSize size) {
	const VkCommandBufferAllocateInfo allocInfo{
	    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
	    .commandPool = mDevice.getCommandPool(),
	    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
	    .commandBufferCount = 1,
	};

	vkAllocateCommandBuffers(mDevice.getDevice(), &allocInfo, &pCommandBuffer);

	constexpr VkCommandBufferBeginInfo beginInfo{.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	                                             .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT};

	vkBeginCommandBuffer(pCommandBuffer, &beginInfo);

	const VkBufferCopy copyRegion{.size = size};
	vkCmdCopyBuffer(pCommandBuffer, *srcBuffer, *dstBuffer, 1, &copyRegion);

	vkEndCommandBuffer(pCommandBuffer);

	const VkSubmitInfo submitInfo{
	    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
	    .commandBufferCount = 1,
	    .pCommandBuffers = &pCommandBuffer,
	};

	vkQueueSubmit(mDevice.getGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(mDevice.getGraphicsQueue());

	vkFreeCommandBuffers(mDevice.getDevice(), mDevice.getCommandPool(), 1, &pCommandBuffer);
}
}  // namespace vke