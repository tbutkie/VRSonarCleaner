#pragma once

#include <vector>
#include <random>

#include "glm.hpp"

#include <Eigen/Dense>

#include "DataVolume.h"

class VectorFieldGenerator :
	public DataVolume
{
public:	
	VectorFieldGenerator(glm::vec3 pos, glm::quat rot, glm::vec3 dims);
	~VectorFieldGenerator();

	void setGridResolution(unsigned int res);
	void setGaussianShape(float guassian);

	void createRandomControlPoints(unsigned int nControlPoints);
	void setControlPoint(glm::vec3 pos, glm::vec3 dir);
	void clearControlPoints();
	std::vector<std::pair<glm::vec3, glm::vec3>> getControlPoints();

	void generate();

	float streamRibbonRotation(glm::vec3 domainPt, float delta);
	std::vector<glm::vec3> getStreamline(glm::vec3 pos, float propagation_unit, int propagation_max_units, float terminal_speed, bool clipToDomain = true);

	float divergence(glm::vec3 domainPt, float delta);
	glm::vec3 curl(glm::vec3 domainPt, float delta);
	glm::mat3 jacobian(glm::vec3 domainPt, float delta);
	bool findZeroNewtonRaphson(glm::vec3 domainStartPt, float delta, int maxSteps, float stopVelocity, float stopStepDelta, std::vector<glm::vec3> &pLines);
	float lambda2(glm::vec3 domainPt, float delta);

	glm::vec3 eulerForward(glm::vec3 pos, float delta);
	glm::vec3 rk4(glm::vec3 pos, float delta);

	bool checkSphereAdvection(float dt, float totalTime, glm::vec3 sphereCenter, float sphereRadius, float &timeToAdvect, float &distanceToAdvect, float &totalDistance, glm::vec3 &exitPoint);
	std::vector<std::vector<glm::vec3>> getAdvectedParticles(int numParticles, float dt, float totalTime);

	bool save(std::string path, bool verbose = true);

private:
	struct ControlPoint {
		glm::vec3 pos;
		glm::vec3 dir;
	};

private:	
	std::mt19937 m_RNG; // Mersenne Twister
	std::uniform_real_distribution<float> m_Distribution;

	std::vector<ControlPoint> m_vControlPoints;

	int m_iGridResolution;
	float m_fGaussianShape;
	Eigen::MatrixXf m_matControlPointKernel;
	Eigen::VectorXf m_vCPUVals, m_vCPVVals, m_vCPWVals;
	Eigen::VectorXf m_vLambdaX, m_vLambdaY, m_vLambdaZ;
	std::vector<std::vector<std::vector<std::pair<glm::vec3, glm::vec3>>>> m_v3DGridPairs;

private:
	void solveLUdecomp();
	glm::vec3 interpolate(glm::vec3 pt);
	bool inBounds(glm::vec3 pos);
	float gaussianBasis(float r, float eta);
	Eigen::MatrixXf jacobianEigen(glm::vec3 domainPt, float delta);
};

