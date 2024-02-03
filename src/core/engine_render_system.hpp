//
// Created by zphrfx on 03/02/2024.
//

#pragma once

#include "engine_device.hpp"
#include "engine_ecs.hpp"
#include "engine_pipeline.hpp"

namespace vke {
class VkEngineRenderSystem {
   public:
	VkEngineRenderSystem(VkEngineDevice& device, const VkRenderPass* renderPass);

	~VkEngineRenderSystem();

	VkEngineRenderSystem(const VkEngineRenderSystem&) = delete;

	VkEngineRenderSystem& operator=(const VkEngineRenderSystem&) = delete;

	void renderGameObjects(const VkCommandBuffer* commandBuffer, std::vector<VkEngineGameObjects>& objects) const;


   private:
	void createPipelineLayout();
	void createPipeline(const VkRenderPass* renderPass);


	VkEngineDevice& mVkDevice;
	std::unique_ptr<VkEnginePipeline> pVkPipeline = nullptr;
	VkPipelineLayout pVkPipelineLayout = VK_NULL_HANDLE;
};
}  // namespace vke
