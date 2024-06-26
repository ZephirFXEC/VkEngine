//
// Created by zphrfx on 13/05/2024.
//
#pragma once
#include <utils/types.hpp>

#include "engine_ecs.hpp"
#include "engine_window.hpp"


namespace vke {
class KeyboardController {
   public:
	struct KeyMapping {
		int moveLeft = GLFW_KEY_A;
		int moveRight = GLFW_KEY_D;
		int moveForward = GLFW_KEY_W;
		int moveBackward = GLFW_KEY_S;
		int moveUp = GLFW_KEY_E;
		int moveDown = GLFW_KEY_Q;
		int lookLeft = GLFW_KEY_LEFT;
		int lookRight = GLFW_KEY_RIGHT;
		int lookUp = GLFW_KEY_UP;
		int lookDown = GLFW_KEY_DOWN;
	};

	void moveInPlaneXZ(GLFWwindow* pwindow, float dt, VkEngineGameObjects& gameObjects) const;


   private:
	KeyMapping mKeyMapping{};
	float moveSpeed = 3.0f;
	float lookSpeed = 1.5f;
};
}  // namespace vke
