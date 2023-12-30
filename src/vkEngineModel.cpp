//
// Created by Enzo Crema on 21/12/2023.
//

#include "vkEngineModel.hpp"


#include "utils/bufferUtils.hpp"


namespace vke {
VkEngineModel::VkEngineModel(const VkEngineDevice& device,const std::shared_ptr<VkEngineSwapChain>& swapchain, const Vertex* vertices, const uint32_t vCount,
                             const uint32_t* indices, const uint32_t iCount)

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

	createBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
	             stagingBufferMemory);

	void* mappedData = nullptr;

#ifdef USE_VMA
	VK_CHECK(vmaMapMemory(mDevice.getAllocator(), stagingBufferMemory, &mappedData));
#else
	VK_CHECK(vkMapMemory(mDevice.getDevice(), stagingBufferMemory, 0, bufferSize, 0, &mappedData));
#endif

	memcpy(mappedData, data, static_cast<size_t>(bufferSize));

#ifdef USE_VMA
	vmaUnmapMemory(mDevice.getAllocator(), stagingBufferMemory);
#else
	vkUnmapMemory(mDevice.getDevice(), stagingBufferMemory);
#endif

	createBuffer(bufferSize, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	             VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, buffer, bufferMemory);

	copyBuffer(&stagingBuffer, &buffer, bufferSize);

#ifdef USE_VMA
	vmaDestroyBuffer(mDevice.getAllocator(), stagingBuffer, stagingBufferMemory);
#else
	vkDestroyBuffer(mDevice.getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(mDevice.getDevice(), stagingBufferMemory, nullptr);
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
	    .usage = VMA_MEMORY_USAGE_AUTO	,
	};

	VK_CHECK(vmaCreateBuffer(mDevice.getAllocator(), &bufferInfo, &allocInfo, &buffer, &bufferMemory, nullptr));

#else
	VK_CHECK(vkCreateBuffer(mDevice.getDevice(), &bufferInfo, nullptr, &buffer));

	VkMemoryRequirements memRequirements{};
	vkGetBufferMemoryRequirements(mDevice.getDevice(), buffer, &memRequirements);

	const VkMemoryAllocateInfo allocInfo{
	    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
	    .allocationSize = memRequirements.size,
	    .memoryTypeIndex = mDevice.findMemoryType(memRequirements.memoryTypeBits, properties)};

	VK_CHECK(vkAllocateMemory(mDevice.getDevice(), &allocInfo, nullptr, &bufferMemory));


	VK_CHECK(vkBindBufferMemory(mDevice.getDevice(), buffer, bufferMemory, 0));
#endif
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