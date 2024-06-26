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
	VkEngineRenderSystem(std::shared_ptr<VkEngineDevice> device, VkRenderPass renderPass);

	~VkEngineRenderSystem();

	VkEngineRenderSystem(const VkEngineRenderSystem&) = delete;

	VkEngineRenderSystem& operator=(const VkEngineRenderSystem&) = delete;

	void renderGameObjects(const VkCommandBuffer* commandBuffer, const std::vector<VkEngineGameObjects>& objects,
	                       const VkEngineCamera& camera) const;

	const std::unique_ptr<VkEnginePipeline>& getPipeline() const { return pVkPipeline;}

   private:
	void createPipelineLayout();
	void createPipeline(VkRenderPass renderPass);


	std::shared_ptr<VkEngineDevice> mVkDevice{};
	std::unique_ptr<VkEnginePipeline> pVkPipeline{};
	VkPipelineLayout pVkPipelineLayout = VK_NULL_HANDLE;
};
}  // namespace vke
