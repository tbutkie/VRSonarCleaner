#include "VectorFieldGenerator.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

VectorFieldGenerator::VectorFieldGenerator()
	: m_iGridResolution(10u)
	, m_fGaussianShape(1.f)
{
	m_RNG.seed(std::random_device()());

	m_Distribuion = std::uniform_real_distribution<float>(-1.f, 1.f);
}

VectorFieldGenerator::~VectorFieldGenerator()
{
}

void VectorFieldGenerator::setGridResolution(unsigned int res)
{
	m_iGridResolution = res;
}

void VectorFieldGenerator::setGaussianShape(float gaussian)
{
	m_fGaussianShape = gaussian;
}

void VectorFieldGenerator::createRandomControlPoints(unsigned int nControlPoints)
{
	clearControlPoints();

	for (unsigned int i = 0u; i < nControlPoints; ++i)
		setControlPoint(glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG)) , glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG)));
}

void VectorFieldGenerator::setControlPoint(glm::vec3 pos, glm::vec3 dir)
{
	m_vControlPoints.push_back(ControlPoint({ pos, dir }));
}

void VectorFieldGenerator::clearControlPoints()
{
	if (m_vControlPoints.size() > 0u)
		m_vControlPoints.clear();
}

void VectorFieldGenerator::generate()
{
	solveLUdecomp();

	m_v3DGridPairs.clear();

	// grid cell size when discretizing [-1, 1] cube
	float cellSize = (2.f / static_cast<float>(m_iGridResolution - 1));

	// create a regular 3D grid at the given resolution and interpolate the field at its nodes
	for (int i = 0; i < m_iGridResolution; ++i)
	{
		std::vector<std::vector<std::pair<glm::vec3, glm::vec3>>> frame;
		for (int j = 0; j < m_iGridResolution; ++j)
		{
			std::vector<std::pair<glm::vec3, glm::vec3>> row;
			for (int k = 0; k < m_iGridResolution; ++k)
			{
				// calculate grid point position
				glm::vec3 point;
				point.x = -1.f + k * cellSize;
				point.y = -1.f + j * cellSize;
				point.z = -1.f + i * cellSize;

				glm::vec3 flowHere = interpolate(point);

				row.push_back(std::pair<glm::vec3, glm::vec3>(point, flowHere));
			}
			frame.push_back(row);
		}
		m_v3DGridPairs.push_back(frame);
	}
}

void VectorFieldGenerator::solveLUdecomp()
{
	int nControlPoints = static_cast<int>(m_vControlPoints.size());

	m_matControlPointKernel = Eigen::MatrixXf(nControlPoints, nControlPoints);
	m_vCPXVals = Eigen::VectorXf(nControlPoints);
	m_vCPYVals = Eigen::VectorXf(nControlPoints);
	m_vCPZVals = Eigen::VectorXf(nControlPoints);

	for (int i = 0; i < nControlPoints; ++i)
	{
		// store each vector component of the control point value
		m_vCPXVals(i) = m_vControlPoints[i].dir.x;
		m_vCPYVals(i) = m_vControlPoints[i].dir.y;
		m_vCPZVals(i) = m_vControlPoints[i].dir.z;

		// fill in the distance matrix kernel entries for this control point
		m_matControlPointKernel(i, i) = 1.f;
		for (int j = i - 1; j >= 0; --j)
		{
			float r = glm::length(m_vControlPoints[i].pos - m_vControlPoints[j].pos);
			float gaussian = gaussianBasis(r, m_fGaussianShape);
			m_matControlPointKernel(i, j) = m_matControlPointKernel(j, i) = gaussian;
		}
	}

	// solve for lambda coefficients in each (linearly independent) dimension using LU decomposition of distance matrix
	//     -the lambda coefficients are the interpolation weights for each control(/interpolation data) point
	m_vLambdaX = m_matControlPointKernel.fullPivLu().solve(m_vCPXVals);
	m_vLambdaY = m_matControlPointKernel.fullPivLu().solve(m_vCPYVals);
	m_vLambdaZ = m_matControlPointKernel.fullPivLu().solve(m_vCPZVals);
}

glm::vec3 VectorFieldGenerator::interpolate(glm::vec3 pt)
{
	// find interpolated 3D vector by summing influence from each CP via radial basis function (RBF)
	glm::vec3 outVec(0.f);
	for (int m = 0; m < static_cast<int>(m_vControlPoints.size()); ++m)
	{
		float r = glm::length(pt - m_vControlPoints[m].pos);
		float gaussian = gaussianBasis(r, m_fGaussianShape);

		// interpolate each separate (linearly independent) component of our 3D vector to get CP influence
		float interpX = m_vLambdaX[m] * gaussian;
		float interpY = m_vLambdaY[m] * gaussian;
		float interpZ = m_vLambdaZ[m] * gaussian;

		// add CP influence to resultant vector
		outVec += glm::vec3(interpX, interpY, interpZ);
	}

	return outVec;
}

std::vector<std::vector<glm::vec3>> VectorFieldGenerator::getAdvectedParticles(int numParticles, float dt, float totalTime)
{
	std::vector<std::vector<glm::vec3>> ret;
	std::vector<glm::vec3> seedPoints;

	for (int i = 0; i < numParticles; ++i)
		seedPoints.push_back(glm::vec3(m_Distribuion(m_RNG), m_Distribuion(m_RNG), m_Distribuion(m_RNG)));
	
	for (auto &pt : seedPoints)
	{
		std::vector<glm::vec3> particlePath;
		particlePath.push_back(pt);

		for (float i = 0.f; i < totalTime; i += dt)
		{
			// advect point by one timestep to get new point
			glm::vec3 newPt = pt + dt * interpolate(pt);

			if (abs(newPt.x) > 1.f ||
				abs(newPt.y) > 1.f ||
				abs(newPt.z) > 1.f)
			{
				glm::vec3 clippedPt = newPt;
				clippedPt.x = fmax(fmin(clippedPt.x, 1.f), -1.f);
				clippedPt.y = fmax(fmin(clippedPt.y, 1.f), -1.f);
				clippedPt.z = fmax(fmin(clippedPt.z, 1.f), -1.f);

				particlePath.push_back(clippedPt);
				break;
			}

			particlePath.push_back(newPt);

			pt = newPt;
		}

		ret.push_back(particlePath);
	}

	return ret;
}

// Taken from http://stackoverflow.com/questions/5883169/intersection-between-a-line-and-a-sphere
glm::vec3 lineSphereIntersection(glm::vec3 linePoint0, glm::vec3 linePoint1, glm::vec3 circleCenter, float circleRadius)
{
	// http://www.codeproject.com/Articles/19799/Simple-Ray-Tracing-in-C-Part-II-Triangles-Intersec

	float cx = circleCenter.x;
	float cy = circleCenter.y;
	float cz = circleCenter.z;

	float px = linePoint0.x;
	float py = linePoint0.y;
	float pz = linePoint0.z;

	float vx = linePoint1.x - px;
	float vy = linePoint1.y - py;
	float vz = linePoint1.z - pz;

	float A = vx * vx + vy * vy + vz * vz;
	float B = 2.f * (px * vx + py * vy + pz * vz - vx * cx - vy * cy - vz * cz);
	float C = px * px - 2.f * px * cx + cx * cx + py * py - 2.f * py * cy + cy * cy +
		pz * pz - 2.f * pz * cz + cz * cz - circleRadius * circleRadius;

	// discriminant
	float D = B * B - 4.f * A * C;

	if (D < 0.f)
		return glm::vec3(0.f);

	float t1 = (-B - sqrtf(D)) / (2.f * A);

	glm::vec3 solution1(linePoint0.x * (1.f - t1) + t1 * linePoint1.x,
		linePoint0.y * (1.f - t1) + t1 * linePoint1.y,
		linePoint0.z * (1.f - t1) + t1 * linePoint1.z);

	if (D == 0.f)
		return solution1;

	float t2 = (-B + sqrtf(D)) / (2.f * A);
	glm::vec3 solution2(linePoint0.x * (1.f - t2) + t2 * linePoint1.x,
		linePoint0.y * (1.f - t2) + t2 * linePoint1.y,
		linePoint0.z * (1.f - t2) + t2 * linePoint1.z);

	// prefer a solution that's on the line segment itself

	if (abs(t1 - 0.5f) < abs(t2 - 0.5f))
		return solution1;

	return solution2;
}

bool VectorFieldGenerator::checkSphereAdvection(
	float dt,
	float totalTime,
	glm::vec3 sphereCenter,
	float sphereRadius,
	float &timeToAdvectSphere,
	float &distanceToAdvectSphere,
	float &totalAdvectionDistance,
	glm::vec3 &exitPoint
)
{
	bool advected = false;
	timeToAdvectSphere = -1.f;
	float distanceCounter = distanceToAdvectSphere = 0.f;
	glm::vec3 pt = exitPoint = sphereCenter; // start at the center of the field
	for (float i = 0.f; i < totalTime; i += dt)
	{
		// advect point by one timestep to get new point
		glm::vec3 newPt = pt + dt * interpolate(pt);

		distanceCounter += glm::length(newPt - pt);

		if (glm::length(newPt - sphereCenter) >= sphereRadius && !advected)
		{
			advected = true;
			timeToAdvectSphere = i;
			distanceToAdvectSphere = distanceCounter;

			exitPoint = lineSphereIntersection(pt, newPt, glm::vec3(0.f), sphereRadius);
		}

		pt = newPt;
	}

	totalAdvectionDistance = distanceCounter;

	return advected;
}

float VectorFieldGenerator::gaussianBasis(float radius, float eta)
{
	return exp(-(eta * radius * radius));
}

bool VectorFieldGenerator::save(std::string path, bool verbose)
{
	FILE *exportFile;

	if (verbose) printf("VecFieldGen: Opening FlowGrid %s for export\n", path.c_str());
	fopen_s(&exportFile, path.c_str(), "wb");

	if (exportFile == NULL)
	{
		if (verbose) printf("VecFieldGen: Unable to open FlowGrid export file!");
		return false;
	}

	//xyz min max values are the coordinates, so for our purposes they can be whatever, like -1 to 1 or 0 to 32 etc, the viewer should stretch everything to the same size anyways, using 1 to 32 might be easiest see with depthValues comment below
	//x y and z cells would be 32

	float xMin, xMax, yMin, yMax, zMin, zMax;
	xMin = yMin = zMin = 1.f;
	xMax = yMax = zMax = static_cast<float>(m_iGridResolution);
	int xCells, yCells, zCells;
	xCells = yCells = zCells = m_iGridResolution;
	int numTimesteps = 1;

	fwrite(&xMin, sizeof(float), 1, exportFile);
	fwrite(&xMax, sizeof(float), 1, exportFile);
	fwrite(&xCells, sizeof(int), 1, exportFile);
	fwrite(&yMin, sizeof(float), 1, exportFile);
	fwrite(&yMax, sizeof(float), 1, exportFile);
	fwrite(&yCells, sizeof(int), 1, exportFile);
	fwrite(&zMin, sizeof(float), 1, exportFile);
	fwrite(&zMax, sizeof(float), 1, exportFile);
	fwrite(&zCells, sizeof(int), 1, exportFile);
	fwrite(&numTimesteps, sizeof(int), 1, exportFile);

	//write out evenly spaced depth values, e.g. 1-32 
	for (int i = 1; i <= zCells; i++)
	{
		float depthVal = static_cast<float>(i);
		fwrite(&depthVal, sizeof(float), 1, exportFile);
	}

	//shouldnt matter since just one timestep, just write out 0
	for (int i = 0; i < numTimesteps; i++)
	{
		float n = 0.f;
		fwrite(&n, sizeof(float), 1, exportFile);
	}

	for (int x = 0; x < m_iGridResolution; x++)
	{
		for (int y = 0; y < m_iGridResolution; y++)
		{
			for (int z = 0; z < m_iGridResolution; z++)
			{
				glm::vec3 dir = m_v3DGridPairs[z][y][x].second;

				// Change from +y up to +z up
				float u, v, w;
				u = dir.x;  // EAST
				v = dir.y; // NORTH
				w = dir.z;  // UP (SKY)

				int one = 1;
				fwrite(&one, sizeof(int), 1, exportFile); //just write out 1 (true) for all
				fwrite(&u, sizeof(float), 1, exportFile); // U
				fwrite(&v, sizeof(float), 1, exportFile); // V
				fwrite(&w, sizeof(float), 1, exportFile); // W
			}//end for z
		}//end for y
	}//end for x

	fclose(exportFile);

	if (verbose) printf("VecFieldGen: Exported FlowGrid to %s\n", path.c_str());


	using namespace std::experimental::filesystem::v1;
	std::string metaFileName = path + ".cp";
	std::ofstream metaFile;

	if (verbose) printf("VecFieldGen: Opening metafile %s\n", metaFileName.c_str());

	metaFile.open(metaFileName);

	if (!metaFile.is_open())
	{
		if (verbose) printf("VecFieldGen: Unable to open FlowGrid metadata export file!");
		return false;
	}

	// TODO: Need to scale the points and directions (lambdas too? will a uniform scaling transformation mess up the weights?)
	//       from [-1, 1] to [0, gridResolution)
	float cellSize = (2.f / static_cast<float>(m_iGridResolution - 1));
	for (int i = 0; i < m_vControlPoints.size(); ++i)
	{
		metaFile << "CP" << i << "_POINT," << m_vControlPoints[i].pos.x << "," << m_vControlPoints[i].pos.y << "," << m_vControlPoints[i].pos.z << std::endl;
		metaFile << "CP" << i << "_DIRECTION," << m_vControlPoints[i].dir.x << "," << m_vControlPoints[i].dir.y << "," << m_vControlPoints[i].dir.z << std::endl;
		metaFile << "CP" << i << "_LAMBDA," << m_vLambdaX[i] << "," << m_vLambdaY[i] << "," << m_vLambdaZ[i] << std::endl;
	}

	metaFile.close();

	if (verbose) printf("VecFieldGen: Exported FlowGrid metadata file to %s\n", metaFileName.c_str());

	return true;
}
