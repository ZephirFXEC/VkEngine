//
// Created by zphrfx on 03/02/2024.
//

#include "engine_camera.hpp"

namespace vke {
void VkEngineCamera::setOrthographicProjection(const float left, const float right, const float top, const float bottom,
                                               const float near, const float far) {
	mProjectionMatrix = glm::mat4{1.0f};
	mProjectionMatrix[0][0] = 2.f / (right - left);
	mProjectionMatrix[1][1] = 2.f / (bottom - top);
	mProjectionMatrix[2][2] = 1.f / (far - near);
	mProjectionMatrix[3][0] = -(right + left) / (right - left);
	mProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
	mProjectionMatrix[3][2] = -near / (far - near);
}

void VkEngineCamera::setPerspectiveProjection(const float fovy, const float aspect, const float zNear,
                                              const float zFar) {
	assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
	const float tanHalfFovy = glm::tan(fovy / 2.f);
	mProjectionMatrix = glm::mat4{0.0f};
	mProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
	mProjectionMatrix[1][1] = 1.f / (tanHalfFovy);
	mProjectionMatrix[2][2] = zFar / (zFar - zNear);
	mProjectionMatrix[2][3] = 1.f;
	mProjectionMatrix[3][2] = -(zFar * zNear) / (zFar - zNear);
}
}  // namespace vke