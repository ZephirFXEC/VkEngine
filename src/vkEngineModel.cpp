//
// Created by Enzo Crema on 21/12/2023.
//

#include "vkEngineModel.hpp"

#include "utils/bufferUtils.hpp"

namespace vke {
VkEngineModel::VkEngineModel(const VkEngineDevice& device, const std::shared_ptr<VkEngineSwapChain>& swapchain,
                             const Vertex* vertices, const uint32_t vCount, const uint32_t* indices,
                             const uint32_t iCount)

    : mIndexCount{iCount}, mDevice{device}, mSwapChain{swapchain} {
	createIndexBuffers(indices, iCount);
	createVertexBuffers(vertices, vCount);
}

VkEngineModel::~VkEngineModel() {
	destroyBuffer(mVertexBuffer);
	destroyBuffer(mIndexBuffer);
}

void VkEngineModel::destroyBuffer(const DataBuffer& buffer) const {
	vmaDestroyBuffer(mDevice.getAllocator(), buffer.pDataBuffer, buffer.pDataBufferMemory);
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
                                   VkBuffer& buffer, VmaAllocation& bufferMemory) {
	const VkDeviceSize bufferSize = sizeof(T) * dataSize;

	VkBuffer stagingBuffer = nullptr;
	VmaAllocation stagingBufferMemory = nullptr;

	BufferUtils::createModelBuffer(mDevice, bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, stagingBufferMemory, VMA_MEMORY_USAGE_CPU_TO_GPU);

	void* mappedData = nullptr;
	VK_CHECK(vmaMapMemory(mDevice.getAllocator(), stagingBufferMemory, &mappedData));

	memcpy(mappedData, data, static_cast<size_t>(bufferSize));

	vmaUnmapMemory(mDevice.getAllocator(), stagingBufferMemory);

	BufferUtils::createModelBuffer(mDevice, bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer, bufferMemory, VMA_MEMORY_USAGE_GPU_ONLY);

	copyBuffer(&stagingBuffer, &buffer, bufferSize);

	vmaDestroyBuffer(mDevice.getAllocator(), stagingBuffer, stagingBufferMemory);
}

void VkEngineModel::createVertexBuffers(const Vertex* vertices, const size_t vertexCount) {
	createVkBuffer(vertices, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mVertexBuffer.pDataBuffer,
	               mVertexBuffer.pDataBufferMemory);
}

void VkEngineModel::createIndexBuffers(const uint32_t* indices, const size_t indexCount) {
	createVkBuffer(indices, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mIndexBuffer.pDataBuffer,
	               mIndexBuffer.pDataBufferMemory);
}

void VkEngineModel::copyBuffer(const VkBuffer* const srcBuffer, const VkBuffer* const dstBuffer,
                               const VkDeviceSize size) {
	BufferUtils::beginSingleTimeCommands(mDevice.getDevice(), mSwapChain->getCommandPool(), pCommandBuffer);

	const VkBufferCopy copyRegion{.size = size};
	vkCmdCopyBuffer(pCommandBuffer, *srcBuffer, *dstBuffer, 1, &copyRegion);

	BufferUtils::endSingleTimeCommands(mDevice.getDevice(), mSwapChain->getCommandPool(), pCommandBuffer,
	                                   mDevice.getGraphicsQueue());
}
}  // namespace vke