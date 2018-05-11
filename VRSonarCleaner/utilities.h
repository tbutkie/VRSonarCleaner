#pragma once

#include <vector>
#include <glm.hpp>

namespace ccomutils
{
	void dprintf(const char *fmt, ...);

	std::vector<glm::vec3> transformStereoscopicPoints(
		glm::vec3 centerOfProjA,
		glm::vec3 centerOfProjB,
		glm::vec3 viewPosA,
		glm::vec3 viewPosB,
		glm::vec3 screenCtr,
		glm::vec3 screenNorm,
		const std::vector<glm::vec3> pts
	);

	std::vector<glm::vec3> getScreenIntersections(
		glm::vec3 centerOfProjection,
		glm::vec3 screenCtr,
		glm::vec3 screenNorm,
		const std::vector<glm::vec3> pts
	);

	bool LineLineIntersect(
		glm::vec3 p1,
		glm::vec3 p2,
		glm::vec3 p3,
		glm::vec3 p4,
		glm::vec3* pa,
		glm::vec3* pb,
		float* mua,
		float* mub
	);


	glm::mat4 getBillBoardTransform(
		const glm::vec3 &pos,
		const glm::vec3 &viewPos,
		const glm::vec3 &up,
		bool lockToUpVector
	);

	glm::mat4 getUnprojectionMatrix(
		glm::mat4 &proj,
		glm::mat4 &view,
		glm::mat4 &model,
		glm::ivec4 &vp
	);
}