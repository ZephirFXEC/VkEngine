//
// Created by zphrfx on 03/02/2024.
//

#include "engine_render_system.hpp"

#include <imgui.h>

#include <glm/glm.hpp>

#include "utils/logger.hpp"
#include "utils/types.hpp"

namespace vke {
struct PushConstants {
	glm::mat4 transform{1.f};
	alignas(16) glm::vec3 color{};
};

VkEngineRenderSystem::VkEngineRenderSystem(const VkEngineDevice& device, const VkRenderPass renderPass)
    : mVkDevice(device) {
	createPipelineLayout();
	createPipeline(renderPass);
}

VkEngineRenderSystem::~VkEngineRenderSystem() {
	vkDestroyPipelineLayout(mVkDevice.getDevice(), pVkPipelineLayout, nullptr);
}


void VkEngineRenderSystem::createPipelineLayout() {
	static constexpr VkPushConstantRange pushConstantRange{
	    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
	    .offset = 0,
	    .size = sizeof(PushConstants),
	};

	constexpr VkPipelineLayoutCreateInfo pipelineLayoutInfo{.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
	                                                        .pNext = nullptr,
	                                                        .setLayoutCount = 0,
	                                                        .pSetLayouts = nullptr,
	                                                        .pushConstantRangeCount = 1,
	                                                        .pPushConstantRanges = &pushConstantRange};

	VK_CHECK(vkCreatePipelineLayout(mVkDevice.getDevice(), &pipelineLayoutInfo, nullptr, &pVkPipelineLayout));
}

void VkEngineRenderSystem::createPipeline(const VkRenderPass renderPass) {
	if (pVkPipeline) {
		throw std::runtime_error("Cannot create pipeline before pipeline layout");
	}

	PipelineConfigInfo pipelineConfig{};
	pipelineConfig.renderPass = renderPass;
	pipelineConfig.pipelineLayout = pVkPipelineLayout;

	pVkPipeline =
	    std::make_unique<VkEnginePipeline>(mVkDevice, "C:/Users/zphrfx/Desktop/vkEngine/shaders/simple.vert.spv",
	                                       "C:/Users/zphrfx/Desktop/vkEngine/shaders/simple.frag.spv", pipelineConfig);
}


void VkEngineRenderSystem::renderGameObjects(const VkCommandBuffer* const commandBuffer,
                                             const std::vector<VkEngineGameObjects>& objects,
                                             const VkEngineCamera& camera) const {
	ImGui::Render();

	pVkPipeline->bind(commandBuffer);

	const glm::mat4x4 projectionView = camera.getProjectionMatrix() * camera.getViewMatrix();

	for (const auto& gameObject : objects) {
		const PushConstants pushConstants{
		    .transform = projectionView * gameObject.mTransform.mat4(),
		    .color = gameObject.mColor,
		};

		vkCmdPushConstants(*commandBuffer, pVkPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
		                   0, sizeof(PushConstants), &pushConstants);

		gameObject.pModel->bind(commandBuffer);
		gameObject.pModel->draw(commandBuffer);
	}
}

}  // namespace vke