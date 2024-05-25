//
// Created by zphrfx on 03/02/2024.
//

#include "engine_camera.hpp"

#include <glm/ext/scalar_constants.hpp>

namespace vke {
void VkEngineCamera::setOrthographicProjection(const float left, const float right, const float top, const float bottom,
                                               const float near, const float far) {
	mProjectionMatrix = glm::mat4{1.0f};
	mProjectionMatrix[0][0] = 2.f / (right - left);
	mProjectionMatrix[1][1] = 2.f / (top - bottom);
	mProjectionMatrix[2][2] = 1.f / (far - near);
	mProjectionMatrix[3][0] = -(right + left) / (right - left);
	mProjectionMatrix[3][1] = -(bottom + top) / (top - bottom);
	mProjectionMatrix[3][2] = -near / (far - near);
}
void VkEngineCamera::setViewDirection(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& up) {
	const glm::vec3 w{glm::normalize(direction)};
	const glm::vec3 u{glm::normalize(glm::cross(w, up))};
	const glm::vec3 v{glm::cross(w, u)};

	viewMatrix = glm::mat4{1.f};
	viewMatrix[0][0] = u.x;
	viewMatrix[1][0] = u.y;
	viewMatrix[2][0] = u.z;
	viewMatrix[0][1] = v.x;
	viewMatrix[1][1] = v.y;
	viewMatrix[2][1] = v.z;
	viewMatrix[0][2] = w.x;
	viewMatrix[1][2] = w.y;
	viewMatrix[2][2] = w.z;
	viewMatrix[3][0] = -glm::dot(u, position);
	viewMatrix[3][1] = -glm::dot(v, position);
	viewMatrix[3][2] = -glm::dot(w, position);
}
void VkEngineCamera::setViewTarget(const glm::vec3& position, const glm::vec3& target, const glm::vec3& up) {
	setViewDirection(position, target - position, up);
}
void VkEngineCamera::setViewXYZ(const glm::vec3& position, const glm::vec3& rotation) {
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const glm::vec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
	const glm::vec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
	const glm::vec3 w{(c2 * s1), (-s2), (c1 * c2)};
	viewMatrix = glm::mat4{1.f};
	viewMatrix[0][0] = u.x;
	viewMatrix[1][0] = u.y;
	viewMatrix[2][0] = u.z;
	viewMatrix[0][1] = v.x;
	viewMatrix[1][1] = v.y;
	viewMatrix[2][1] = v.z;
	viewMatrix[0][2] = w.x;
	viewMatrix[1][2] = w.y;
	viewMatrix[2][2] = w.z;
	viewMatrix[3][0] = -glm::dot(u, position);
	viewMatrix[3][1] = -glm::dot(v, position);
	viewMatrix[3][2] = -glm::dot(w, position);
}

void VkEngineCamera::setPerspectiveProjection(const float fovy, const float aspect, const float zNear,
                                              const float zFar) {
	assert(glm::abs(aspect - glm::epsilon<float>()) > 0.0f);
	const float tanHalfFovy = glm::tan(fovy / 2.f);
	mProjectionMatrix = glm::mat4{0.0f};
	mProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
	mProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
	mProjectionMatrix[2][2] = zFar / (zFar - zNear);
	mProjectionMatrix[2][3] = 1.f;
	mProjectionMatrix[3][2] = -(zFar * zNear) / (zFar - zNear);
}
}  // namespace vke