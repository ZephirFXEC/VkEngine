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
#ifdef USE_VMA
	vmaDestroyBuffer(mDevice.getAllocator(), buffer.pDataBuffer, buffer.pDataBufferMemory);
#else
	vkDestroyBuffer(mDevice.getDevice(), buffer.pDataBuffer, nullptr);
	vkFreeMemory(mDevice.getDevice(), buffer.pDataBufferMemory, nullptr);
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

	createModelBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, stagingBuffer, stagingBufferMemory);

	void* mappedData = nullptr;
	mapBufferMemory(stagingBufferMemory, bufferSize, &mappedData);

	memcpy(mappedData, data, static_cast<size_t>(bufferSize));

	unmapBufferMemory(stagingBufferMemory);

	createModelBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, buffer, bufferMemory);

	copyBuffer(&stagingBuffer, &buffer, bufferSize);

	destroyBuffer(stagingBuffer, stagingBufferMemory);
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

void VkEngineModel::createModelBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer,
                                      Alloc& bufferMemory) const {
#ifdef USE_VMA
	BufferUtils::createModelBuffer(mDevice, size, usage, buffer, bufferMemory);
#else
	BufferUtils::createModelBuffer(mDevice, size, usage,
	                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer,
	                               bufferMemory);
#endif
}

void VkEngineModel::mapBufferMemory(Alloc& memory, VkDeviceSize size, void** dataPtr) const {
#ifdef USE_VMA
	VK_CHECK(vmaMapMemory(mDevice.getAllocator(), memory, dataPtr));
#else
	VK_CHECK(vkMapMemory(mDevice.getDevice(), memory, 0, size, 0, dataPtr));
#endif
}

void VkEngineModel::unmapBufferMemory(Alloc& memory) const {
#ifdef USE_VMA
	vmaUnmapMemory(mDevice.getAllocator(), memory);
#else
	vkUnmapMemory(mDevice.getDevice(), memory);
#endif
}

void VkEngineModel::destroyBuffer(VkBuffer& buffer, Alloc& memory) const {
#ifdef USE_VMA
	vmaDestroyBuffer(mDevice.getAllocator(), buffer, memory);
#else
	vkDestroyBuffer(mDevice.getDevice(), buffer, nullptr);
	vkFreeMemory(mDevice.getDevice(), memory, nullptr);
#endif
}
}  // namespace vke