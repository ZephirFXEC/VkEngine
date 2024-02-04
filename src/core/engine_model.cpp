//
// Created by Enzo Crema on 21/12/2023.
//

#include "engine_model.hpp"

#include "utils/buffer_utils.hpp"
#include "utils/logger.hpp"
#include "utils/memory.hpp"

namespace vke {
VkEngineModel::VkEngineModel(const VkEngineDevice& device, std::shared_ptr<VkEngineSwapChain> swapchain,
                             const MeshData& meshData)

    : mIndexCount{meshData.iCount}, mDevice{device}, pSwapChain{std::move(swapchain)} {
	VKINFO("Creating model");
	createIndexBuffers(meshData.pIndices, meshData.iCount);
	createVertexBuffers(meshData.pVertices, meshData.vCount);
}

VkEngineModel::~VkEngineModel() {
	vmaDestroyBuffer(mDevice.getAllocator(), mIndexBuffer.pDataBuffer, mIndexBuffer.pDataBufferMemory);
	vmaDestroyBuffer(mDevice.getAllocator(), mVertexBuffer.pDataBuffer, mVertexBuffer.pDataBufferMemory);

	VKINFO("Destroyed model");
}

std::array<VkVertexInputBindingDescription, 1> VkEngineModel::Vertex::getBindingDescriptions() {
	return std::array{VkVertexInputBindingDescription{
	    .binding = 0, .stride = sizeof(Vertex), .inputRate = VK_VERTEX_INPUT_RATE_VERTEX}};
}


std::array<VkVertexInputAttributeDescription, 2> VkEngineModel::Vertex::getAttributeDescriptions() {
	return std::array{

	    VkVertexInputAttributeDescription{
	        .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, mPosition)},

	    VkVertexInputAttributeDescription{
	        .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, mColor)}};
}


void VkEngineModel::bind(const VkCommandBuffer* const commandBuffer) const {
	auto* const buffer = mVertexBuffer.pDataBuffer;
	constexpr std::array<VkDeviceSize, 1> offsets{0};

	vkCmdBindVertexBuffers(*commandBuffer, 0, 1, &buffer, offsets.data());
	vkCmdBindIndexBuffer(*commandBuffer, mIndexBuffer.pDataBuffer, 0, VK_INDEX_TYPE_UINT32);
}


void VkEngineModel::draw(const VkCommandBuffer* const commandBuffer) const {
	vkCmdDrawIndexed(*commandBuffer, mIndexCount, 1, 0, 0, 0);
}


template <typename T>
void VkEngineModel::createVkBuffer(const T* data, const size_t dataSize, const VkBufferUsageFlags usage,
                                   VkBuffer& buffer, VmaAllocation& bufferMemory) {
	const VkDeviceSize bufferSize = sizeof(T) * dataSize;

	const VkBufferCreateInfo bufferInfo{
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = bufferSize,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE
	};

	constexpr VmaAllocationCreateInfo allocCreateInfo{
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE, // device is GPU preferred
	};

	VmaAllocationInfo allocInfo{};
	VK_CHECK(vmaCreateBuffer(mDevice.getAllocator(), &bufferInfo, &allocCreateInfo, &buffer, &bufferMemory, &allocInfo));

	Memory::copyMemory(allocInfo.pMappedData, data, static_cast<size_t>(bufferSize));
}


void VkEngineModel::createVertexBuffers(const Vertex* vertices, const size_t vertexCount) {
	createVkBuffer(vertices, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mVertexBuffer.pDataBuffer,
	               mVertexBuffer.pDataBufferMemory);
}


void VkEngineModel::createIndexBuffers(const u32* indices, const size_t indexCount) {
	createVkBuffer(indices, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mIndexBuffer.pDataBuffer,
	               mIndexBuffer.pDataBufferMemory);
}

}  // namespace vke