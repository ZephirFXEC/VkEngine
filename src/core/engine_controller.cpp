//
// Created by zphrfx on 13/05/2024.
//

#include "engine_controller.hpp"

#include <utils/logger.hpp>
namespace vke {
void KeyboardController::moveInPlaneXZ(GLFWwindow* pwindow, float dt, VkEngineGameObjects& gameObjects) const {

	glm::vec3 rot{0.0f};

	rot.y += static_cast<float>(glfwGetKey(pwindow, mKeyMapping.lookRight) == GLFW_PRESS);
	rot.y -= static_cast<float>(glfwGetKey(pwindow, mKeyMapping.lookLeft) == GLFW_PRESS);
	rot.x += static_cast<float>(glfwGetKey(pwindow, mKeyMapping.lookUp) == GLFW_PRESS);
	rot.x -= static_cast<float>(glfwGetKey(pwindow, mKeyMapping.lookDown) == GLFW_PRESS);

	// Check for null
	if(glm::dot(rot, rot) > glm::epsilon<float>()) {
		gameObjects.mTransform.rotation += glm::normalize(rot) * lookSpeed * dt;
	}

	gameObjects.mTransform.rotation.x = glm::clamp(gameObjects.mTransform.rotation.x, -1.5f, 1.5f);
	gameObjects.mTransform.rotation.y = glm::mod(gameObjects.mTransform.rotation.y, glm::two_pi<float>());

	const float yaw = gameObjects.mTransform.rotation.y;
	const glm::vec3 forward = {glm::sin(yaw), 0.f, glm::cos(yaw)};
	const glm::vec3 right = {forward.z, 0.f, -forward.x};
	constexpr glm::vec3 up = {0.f, -1.f, 0.f};

	glm::vec3 moveDir{0.0f};
	moveDir += static_cast<float>(glfwGetKey(pwindow, mKeyMapping.moveForward) == GLFW_PRESS) * forward;
	moveDir -= static_cast<float>(glfwGetKey(pwindow, mKeyMapping.moveBackward) == GLFW_PRESS) * forward;
	moveDir += static_cast<float>(glfwGetKey(pwindow, mKeyMapping.moveRight) == GLFW_PRESS) * right;
	moveDir -= static_cast<float>(glfwGetKey(pwindow, mKeyMapping.moveLeft) == GLFW_PRESS) * right;
	moveDir += static_cast<float>(glfwGetKey(pwindow, mKeyMapping.moveUp) == GLFW_PRESS) * up;
	moveDir -= static_cast<float>(glfwGetKey(pwindow, mKeyMapping.moveDown) == GLFW_PRESS) * up;

	if(glm::dot(moveDir, moveDir) > glm::epsilon<float>()) {
		gameObjects.mTransform.translation += glm::normalize(moveDir) * moveSpeed * dt;
	}
}
}