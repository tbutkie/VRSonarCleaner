#pragma once

#include <vector>
#include <glm.hpp>

namespace utils
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

	glm::mat4 getViewingFrustum(glm::vec3 eyePos, glm::vec3 screenCenter, glm::vec3 screenNormal, glm::vec3 screenUp, glm::vec2 screenSize);

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

	int getIndexWrapped(int index, int maxIndex, int baseIndex = 0, bool rebaseToZero = false);
	

	///////////////////////////
	//						 //
	//    COLOR UTILITIES    //
	//						 //
	///////////////////////////
	// taken from David H. https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
	namespace color {
		bool rgb2hsv(
			glm::vec3 in, 
			float &hOut, 
			float &sOut, 
			float &vOut
		);

		glm::vec3 hsv2rgb(
			float h, 
			float s, 
			float v
		);

		glm::vec4 str2rgb(std::string color);

		std::string rgb2str(glm::vec4 color);
		std::string rgb2str(glm::vec3 color);
	}
}