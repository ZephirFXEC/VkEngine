//
// Created by zphrfx on 03/02/2024.
//

#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vke {

class VkEngineCamera {
   public:
	void setPerspectiveProjection(float fovy, float aspect, float zNear, float zFar);
	void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);

	const glm::mat4& getProjectionMatrix() const { return mProjectionMatrix; }

   private:
	glm::mat4 mProjectionMatrix{1.f};
};

}  // namespace vke
