//
// Created by zphrfx on 03/02/2024.
//

#pragma once

#include "core/engine_device.hpp"
#include "core/engine_ecs.hpp"
#include "core/engine_pipeline.hpp"
#include "engine_camera.hpp"

namespace vke {
class VkEngineRenderSystem {
   public:
	VkEngineRenderSystem(const VkEngineDevice& device, const VkRenderPass* renderPass);

	~VkEngineRenderSystem();

	VkEngineRenderSystem(const VkEngineRenderSystem&) = delete;

	VkEngineRenderSystem& operator=(const VkEngineRenderSystem&) = delete;

	void renderGameObjects(const VkCommandBuffer* commandBuffer, const std::vector<VkEngineGameObjects>& objects,
	                       const VkEngineCamera& camera) const;


   private:
	void createPipelineLayout();
	void createPipeline(const VkRenderPass* renderPass);


	const VkEngineDevice& mVkDevice;
	std::unique_ptr<VkEnginePipeline> pVkPipeline = nullptr;
	VkPipelineLayout pVkPipelineLayout = VK_NULL_HANDLE;
};
}  // namespace vke
