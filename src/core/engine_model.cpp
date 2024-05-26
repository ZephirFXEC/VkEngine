//
// Created by Enzo Crema on 21/12/2023.
//

#include "engine_model.hpp"

// tinyobjloader
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "utils/buffer_utils.hpp"
#include "utils/logger.hpp"
#include "utils/memory.hpp"



template <>
struct std::hash<vke::VkEngineModel::Vertex> {
	std::size_t operator()(const vke::VkEngineModel::Vertex &vertex) const noexcept  {
		std::size_t seed = 0 ;
		vke::hashCombine(seed, vertex.mPosition, vertex.mColor, vertex.mNormal, vertex.mUV);
		return seed;
	}
};

namespace vke {

VkEngineModel::VkEngineModel(const VkEngineDevice& device, const MeshData& meshData)

    : mIndexCount{meshData.iCount}, mDevice{device} {

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


std::array<VkVertexInputAttributeDescription, 4> VkEngineModel::Vertex::getAttributeDescriptions() {
	return std::array{
	    VkVertexInputAttributeDescription{
	        .location = 0, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, mPosition)},
	    VkVertexInputAttributeDescription{
	        .location = 1, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, mColor)},
		VkVertexInputAttributeDescription{
			.location = 2, .binding = 0, .format = VK_FORMAT_R32G32B32_SFLOAT, .offset = offsetof(Vertex, mNormal)},
		VkVertexInputAttributeDescription{
			.location = 3, .binding = 0, .format = VK_FORMAT_R32G32_SFLOAT, .offset = offsetof(Vertex, mUV)}};
}


void VkEngineModel::bind(const VkCommandBuffer* const commandBuffer) const {
	auto* const buffer = mVertexBuffer.pDataBuffer;
	constexpr std::array<VkDeviceSize, 1> offsets{};

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

	const VkBufferCreateInfo bufferInfo{.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
	                                    .size = bufferSize,
	                                    .usage = usage,
	                                    .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

	constexpr VmaAllocationCreateInfo allocCreateInfo{
	    .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
	    .usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,  // device is GPU preferred
	};

	VmaAllocationInfo allocInfo{};
	VK_CHECK(
	    vmaCreateBuffer(mDevice.getAllocator(), &bufferInfo, &allocCreateInfo, &buffer, &bufferMemory, &allocInfo));

	Memory::copyMemory(allocInfo.pMappedData, data, allocInfo.size);
}


void VkEngineModel::createVertexBuffers(const Vertex* vertices, const size_t vertexCount) {
	createVkBuffer(vertices, vertexCount, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, mVertexBuffer.pDataBuffer,
	               mVertexBuffer.pDataBufferMemory);
}


void VkEngineModel::createIndexBuffers(const u32* indices, const size_t indexCount) {
	createVkBuffer(indices, indexCount, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, mIndexBuffer.pDataBuffer,
	               mIndexBuffer.pDataBufferMemory);
}

std::unique_ptr<VkEngineModel> VkEngineModel::createModelFromFile(const VkEngineDevice& device,
																  const std::string& filepath) {
	MeshData meshData{};
	meshData.loadModel(filepath);

	VKINFO("Creating model from file: {}, Triangles : {} ", filepath, meshData.vCount);
	return std::make_unique<VkEngineModel>(device, meshData);
}


void VkEngineModel::MeshData::loadModel(const std::string& filepath) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string err;

	if(!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
		throw std::runtime_error(warn + err);
	}

	delete[] pVertices;
	delete[] pIndices;

	std::unordered_map<Vertex, u32> uniqueVertices{};
	std::vector<Vertex> vertices{};
	std::vector<u32> indices{};
	for(const auto& shape : shapes) {
		for(const auto& index : shape.mesh.indices) {
			Vertex vertex{};

			if(index.vertex_index > 0) {
				vertex.mPosition = {attrib.vertices[3 * index.vertex_index + 0],
				                    attrib.vertices[3 * index.vertex_index + 1],
				                    attrib.vertices[3 * index.vertex_index + 2]
				};
			}

			if(index.vertex_index > 0) {
				vertex.mColor = {attrib.colors[3 * index.vertex_index + 0],
									attrib.colors[3 * index.vertex_index + 1],
									attrib.colors[3 * index.vertex_index + 2]
				};
			}

			if(index.normal_index > 0) {
				vertex.mNormal = {attrib.normals[3 * index.normal_index + 0],
				                 attrib.normals[3 * index.normal_index + 1],
				                 attrib.normals[3 * index.normal_index + 2]
				};
			}

			if(index.texcoord_index > 0) {
				vertex.mUV = {attrib.texcoords[2 * index.texcoord_index + 0],
								attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}


			if(!uniqueVertices.contains(vertex)) {
				uniqueVertices[vertex] = static_cast<u32>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}


	vCount = static_cast<u32>(vertices.size());
	iCount = static_cast<u32>(indices.size());

	pVertices = Memory::allocMemory<Vertex>(vCount, MEMORY_TAG_ENGINE);
	pIndices = Memory::allocMemory<u32>(iCount, MEMORY_TAG_ENGINE);

	std::ranges::copy(vertices.begin(), vertices.end(), pVertices);
	std::ranges::copy(indices.begin(), indices.end(), pIndices);


}

}  // namespace vke