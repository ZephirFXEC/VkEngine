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
	void setViewDirection(const glm::vec3& position, const glm::vec3& direction,
	                      const glm::vec3& up = glm::vec3{0.f, -1.f, 0.f});
	void setViewTarget(const glm::vec3& position, const glm::vec3& target,
	                   const glm::vec3& up = glm::vec3{0.f, -1.f, 0.f});
	void setViewXYZ(const glm::vec3& position, const glm::vec3& rotation);

	const glm::mat4& getProjectionMatrix() const { return mProjectionMatrix; }
	const glm::mat4& getViewMatrix() const { return viewMatrix; }

   private:
	glm::mat4 mProjectionMatrix{1.f};
	glm::mat4 viewMatrix{1.f};
};

}  // namespace vke
