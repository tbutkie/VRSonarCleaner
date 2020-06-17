#include "SonarPointCloud.h"

#include "GLSLpreamble.h"
#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <numeric>
#include <limits>

#include "laszip_api.h"

SonarPointCloud::SonarPointCloud(ColorScaler * const colorScaler, std::string fileName, SONAR_FILETYPE filetype)
	: Dataset(fileName, (filetype == XYZF || filetype == QIMERA) ? true : false)
	, m_Sonar_Filetype(filetype)
	, m_glVAO(0u)
	, m_glPreviewVAO(0u)
	, m_glPointsBufferVBO(0u)
	, m_pColorScaler(colorScaler)
	, m_iPreviewReductionFactor(10)
	, m_bPointsAllocated(false)
	, m_bEnabled(true)
	, refreshNeeded(true)
	, previewRefreshNeeded(true)
	, m_nPoints(0)
	, colorMode(1) //0=predefined 1=scaled
	, colorScale(2)
	, colorScope(1) //0=global 1=dynamic
	, m_fMinPositionalTPU(std::numeric_limits<float>::max())
	, m_fMaxPositionalTPU(-std::numeric_limits<float>::max())
	, m_fMinDepthTPU(std::numeric_limits<float>::max())
	, m_fMaxDepthTPU(-std::numeric_limits<float>::max())
{
	switch (m_Sonar_Filetype)
	{
	case SonarPointCloud::CARIS:
		m_Future = std::async(std::launch::async, &SonarPointCloud::loadCARISTxt, this);
		break;
	case SonarPointCloud::XYZF:
		m_Future = std::async(std::launch::async, &SonarPointCloud::loadStudyCSV, this);
		break;
	case SonarPointCloud::QIMERA:
		m_Future = std::async(std::launch::async, &SonarPointCloud::loadQimeraTxt, this);
		break;
	case SonarPointCloud::LIDAR_TXT:
		m_Future = std::async(std::launch::async, &SonarPointCloud::loadLIDARTxt, this);
		break;
	case SonarPointCloud::LIDAR_LAS:
		m_Future = std::async(std::launch::async, &SonarPointCloud::loadLIDAR, this);
		break;
	default:
		break;
	}
}

SonarPointCloud::~SonarPointCloud()
{	
}

void SonarPointCloud::initPoints(int numPointsToAllocate)
{
	m_nPoints = numPointsToAllocate;
	
	if (m_bPointsAllocated)
	{
		m_vdvec3RawPointsPositions.clear();
		m_vvec3AdjustedPointsPositions.clear();
		m_vvec4PointsColors.clear();
		m_vuiPointsMarks.clear();
		m_vfPointsDepthTPU.clear();
		m_vfPointsPositionTPU.clear();
	}
	
	m_vdvec3RawPointsPositions.resize(m_nPoints);
	m_vvec3AdjustedPointsPositions.resize(m_nPoints);
	m_vvec3DefaultPointsColors.resize(m_nPoints);
	m_vvec4PointsColors.resize(m_nPoints);
	m_vuiPointsMarks.resize(m_nPoints);
	m_vfPointsDepthTPU.resize(m_nPoints);
	m_vfPointsPositionTPU.resize(m_nPoints);


	m_bPointsAllocated = true;
}

void SonarPointCloud::setPoint(int index, double lonX, double latY, double depth)
{
	glm::dvec3 pt(lonX, latY, depth);
	m_vdvec3RawPointsPositions[index] = pt;
	
	m_vvec4PointsColors[index] = glm::vec4(0.75f, 0.75f, 0.75f, 1.f);

	m_vfPointsDepthTPU[index] = 0.f;
	m_vfPointsPositionTPU[index] = 0.f;
	m_vuiPointsMarks[index] = 0u;
	
	checkNewPosition(pt);
}


void SonarPointCloud::setUncertaintyPoint(int index, double lonX, double latY, double depth, float depthTPU, float positionTPU)
{
	glm::dvec3 pt(lonX, latY, depth); 
	m_vdvec3RawPointsPositions[index] = pt;

	float r, g, b;
	m_pColorScaler->getBiValueScaledColor(depthTPU, positionTPU, &r, &g, &b);
	m_vvec4PointsColors[index] = glm::vec4(r, g, b, 1.f);


	m_vfPointsDepthTPU[index] = depthTPU;
	m_vfPointsPositionTPU[index] = positionTPU;

	m_vuiPointsMarks[index] = 0u;

	checkNewPosition(pt);

	if (depthTPU < m_fMinDepthTPU)
		m_fMinDepthTPU = depthTPU;
	if (depthTPU > m_fMaxDepthTPU)
		m_fMaxDepthTPU = depthTPU;

	if (positionTPU < m_fMinPositionalTPU)
		m_fMinPositionalTPU = positionTPU;
	if (positionTPU > m_fMaxPositionalTPU)
		m_fMaxPositionalTPU = positionTPU;
}


void SonarPointCloud::setColoredPoint(int index, double lonX, double latY, double depth, float r, float g, float b)
{
	glm::dvec3 pt(lonX, latY, depth);
	m_vdvec3RawPointsPositions[index] = pt;

	m_vvec3DefaultPointsColors[index] = glm::vec3(r, g, b);
	m_vvec4PointsColors[index] = glm::vec4(m_vvec3DefaultPointsColors[index], 1.f);

	m_vfPointsDepthTPU[index] = 0.f;
	m_vfPointsPositionTPU[index] = 0.f;
	m_vuiPointsMarks[index] = 0u;

	checkNewPosition(pt);
}


bool SonarPointCloud::loadCARISTxt()
{
	printf("Loading Point Cloud from %s\n", getName().c_str());
		
	FILE *file;
	file = fopen(getName().c_str(), "r");
	if (file == NULL)
	{
		printf("ERROR reading file in %s\n", __FUNCTION__);
		return false;
	}
	else
	{
		//count points
		//skip the first linesToSkip lines
		int skipped = 0;
		int tries = 0;
		char tempChar;
		while (skipped < 1 && tries < 5000) ///1=lines to skip
		{
			tries++;
			tempChar = 'a';
			while (tempChar != '\n')
			{
				tempChar = fgetc(file);
			}
			skipped++;
		}
		printf("Skipped %d characters\n", skipped);

		//now count lines of points
		double x, y, depth;
		int profnum, beamnum;
		float depthTPU, positionTPU, alongAngle, acrossAngle;
		unsigned int numPointsInFile = 0u;
		while (fscanf(file, "%lf,%lf,%lf,%d,%d,%f,%f,%f,%f\n", &x, &y, &depth, &profnum, &beamnum, &depthTPU, &positionTPU, &alongAngle, &acrossAngle) != EOF)  //while another valid entry to load
			numPointsInFile++;

		initPoints(numPointsInFile);
		printf("found %d lines of points\n", numPointsInFile);

		//rewind
		rewind(file);
		//skip the first linesToSkip lines
		skipped = 0;
		tries = 0;
		while (skipped < 1 && tries < 5000) ///1=lines to skip
		{
			tries++;
			tempChar = 'a';
			while (tempChar != '\n')
			{
				tempChar = fgetc(file);
			}
			skipped++;
		}

		//now load lines of points
		GLuint index = 0u;
		double averageDepth =  0.0;
		while (fscanf(file, "%lf,%lf,%lf,%d,%d,%f,%f,%f,%f\n", &x, &y, &depth, &profnum, &beamnum, &depthTPU, &positionTPU, &alongAngle, &acrossAngle) != EOF)  //while another valid entry to load
		{
			setUncertaintyPoint(index++, x, y, depth, depthTPU, positionTPU);
			averageDepth += depth;
		}
		averageDepth /= m_nPoints;

		printf("Loaded %d points\n", index);

		printf("Original Min/Maxes:\n");
		printf("X Min: %f Max: %f\n", getXMin(), getXMax());
		printf("Y Min: %f Max: %f\n", getYMin(), getYMax());
		printf("Depth Min: %f Max: %f\n", getZMin(), getZMax());
		printf("Depth Avg: %f\n", averageDepth);

		fclose(file);
		
		adjustPoints();

		setRefreshNeeded();
	}

	return true;
}

bool SonarPointCloud::loadQimeraTxt()
{
	printf("Loading Study Point Cloud from %s\n", getName().c_str());

	bool rejectedDataset = getName().find("reject") != std::string::npos;

	FILE *file;
	file = fopen(getName().c_str(), "r");
	if (file == NULL)
	{
		printf("ERROR reading file in %s\n", __FUNCTION__);
	}
	else
	{
		//count points
		//skip the first linesToSkip lines
		int skipped = 0;
		int tries = 0;
		char tempChar;
		while (skipped < 1 && tries < 5000) ///1=lines to skip
		{
			tries++;
			tempChar = 'a';
			while (tempChar != '\n')
			{
				tempChar = fgetc(file);
			}
			skipped++;
		}
		printf("Skipped %d characters\n", skipped);

		//now count lines of points
		double x, y, depth;
		unsigned int numPointsInFile = 0u;
		while (fscanf(file, "%lf %lf %lf\n", &x, &y, &depth) != EOF)  //while another valid entry to load
			numPointsInFile++;

		initPoints(numPointsInFile);
		printf("found %d lines of points\n", numPointsInFile);

		//rewind
		rewind(file);
		//skip the first linesToSkip lines
		skipped = 0;
		tries = 0;
		while (skipped < 1 && tries < 5000) ///1=lines to skip
		{
			tries++;
			tempChar = 'a';
			while (tempChar != '\n')
			{
				tempChar = fgetc(file);
			}
			skipped++;
		}

		//now load lines of points
		GLuint index = 0u;
		double averageDepth = 0.0;
		while (fscanf(file, "%lf %lf %lf\n", &x, &y, &depth) != EOF)  //while another valid entry to load
		{
			float conf = rejectedDataset ? 1.f : 0.f;
			setUncertaintyPoint(index++, x, y, depth, conf, conf);
			averageDepth += depth;
			assert(depth < 0.);
		}
		averageDepth /= m_nPoints;

		printf("Loaded %d points\n", index);

		printf("Original Min/Maxes:\n");
		printf("X Min: %f Max: %f\n", getXMin(), getXMax());
		printf("Y Min: %f Max: %f\n", getYMin(), getYMax());
		printf("Depth Min: %f Max: %f\n", getZMin(), getZMax());
		printf("Depth Avg: %f\n", averageDepth);

		fclose(file);

		adjustPoints();

		setRefreshNeeded();
	}

	return true;
}

bool SonarPointCloud::loadLIDARTxt()
{
	printf("Loading LIDAR Point Cloud from %s\n", getName().c_str());
	
	FILE *file;
	file = fopen(getName().c_str(), "r");
	if (file == NULL)
	{
		printf("ERROR reading file in %s\n", __FUNCTION__);
	}
	else
	{
		//count points
		//skip the first linesToSkip lines
		int skipped = 0;
		int tries = 0;
		char tempChar;
		while (skipped < 1 && tries < 5000) ///1=lines to skip
		{
			tries++;
			tempChar = 'a';
			while (tempChar != '\n')
			{
				tempChar = fgetc(file);
			}
			skipped++;
		}
		printf("Skipped %d characters\n", skipped);

		//now count lines of points
		double x, y, height, tmp;
		unsigned int numPointsInFile = 0u;
		while (fscanf(file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n", &x, &y, &height, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp) != EOF)  //while another valid entry to load
			numPointsInFile++;

		initPoints(numPointsInFile);
		printf("found %d lines of points\n", numPointsInFile);

		//rewind
		rewind(file);
		//skip the first linesToSkip lines
		skipped = 0;
		tries = 0;
		while (skipped < 1 && tries < 5000) ///1=lines to skip
		{
			tries++;
			tempChar = 'a';
			while (tempChar != '\n')
			{
				tempChar = fgetc(file);
			}
			skipped++;
		}

		//now load lines of points
		GLuint index = 0u;
		double averageHeight = 0.0;
		while (fscanf(file, "%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf\n", &x, &y, &height, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp, &tmp) != EOF)  //while another valid entry to load
		{
			setUncertaintyPoint(index++, x, y, -height, 0.f, 0.f);
			averageHeight += height;
		}
		averageHeight /= m_nPoints;

		printf("Loaded %d points\n", index);

		printf("Original Min/Maxes:\n");
		printf("X Min: %f Max: %f\n", getXMin(), getXMax());
		printf("Y Min: %f Max: %f\n", getYMin(), getYMax());
		printf("Height Min: %f Max: %f\n", getZMin(), getZMax());
		printf("Height Avg: %f\n", averageHeight);

		fclose(file);

		adjustPoints();

		setRefreshNeeded();
	}

	return true;
}


bool SonarPointCloud::loadLIDAR()
{
	printf("Loading LIDAR Point Cloud from %s\n", getName().c_str());

	Renderer::getInstance().showMessage(std::string("Loading ") + getName());

	if (laszip_load_dll())
	{
		fprintf(stderr, "DLL ERROR: loading LASzip DLL\n");
		return false;
	}

	// get version of LASzip DLL

	laszip_U8 version_major;
	laszip_U8 version_minor;
	laszip_U16 version_revision;
	laszip_U32 version_build;

	if (laszip_get_version(&version_major, &version_minor, &version_revision, &version_build))
	{
		fprintf(stderr, "DLL ERROR: getting LASzip DLL version number\n");
		return false;
	}

	fprintf(stderr, "LASzip DLL v%d.%d r%d (build %d)\n", (int)version_major, (int)version_minor, (int)version_revision, (int)version_build);

	// create the reader

	laszip_POINTER laszip_reader;
	if (laszip_create(&laszip_reader))
	{
		fprintf(stderr, "DLL ERROR: creating laszip reader\n");
		return false;
	}

	// open the reader

	laszip_BOOL is_compressed = 0;
	if (laszip_open_reader(laszip_reader, getName().c_str(), &is_compressed))
	{
		fprintf(stderr, "DLL ERROR: opening laszip reader for '%s'\n", getName().c_str());
		return false;
	}

	fprintf(stderr, "file '%s' is %scompressed\n", getName().c_str(), (is_compressed ? "" : "un"));

	// get a pointer to the header of the reader that was just populated

	laszip_header* header;

	if (laszip_get_header_pointer(laszip_reader, &header))
	{
		fprintf(stderr, "DLL ERROR: getting header pointer from laszip reader\n");
		return false;
	}


	std::cout << "Compressed: " << (is_compressed ? "true" : "false") << std::endl;
	std::cout << "Signature: " << header->file_source_ID << std::endl;
	std::cout << "Points count: " << header->number_of_point_records << std::endl;
	std::cout << "X Min: " << header->min_x << std::endl;
	std::cout << "X Max: " << header->max_x << std::endl;
	std::cout << "Y Min: " << header->min_y << std::endl;
	std::cout << "Y Max: " << header->max_y << std::endl;
	std::cout << "Z Min: " << header->min_z << std::endl;
	std::cout << "Z Max: " << header->max_z << std::endl;

	// how many points does the file have

	laszip_I64 npoints = (header->number_of_point_records ? header->number_of_point_records : header->extended_number_of_point_records);

	// report how many points the file has

	fprintf(stderr, "file '%s' contains %I64d points\n", getName().c_str(), npoints);

	initPoints(npoints);

	// get a pointer to the points that will be read

	laszip_point* point;

	if (laszip_get_point_pointer(laszip_reader, &point))
	{
		fprintf(stderr, "DLL ERROR: getting point pointer from laszip reader\n");
		return false;
	}	

	// read the points

	laszip_I64 p_count = 0;
	double averageHeight = 0.0;

	while (p_count < npoints)
	{
		// read a point

		if (laszip_read_point(laszip_reader))
		{
			fprintf(stderr, "DLL ERROR: reading point %I64d\n", p_count);
		}

		glm::vec3 color(point->rgb[0], point->rgb[1], point->rgb[2]);
		color /= float(1 << 16);

		setColoredPoint(p_count++, point->X, point->Y, -point->Z, color.r, color.g, color.b);
		averageHeight += point->Z;
	}

	fprintf(stderr, "successfully read %I64d points\n", p_count);

	averageHeight /= m_nPoints;

	printf("Loaded %d points\n", npoints);

	printf("Original Min/Maxes:\n");
	printf("X Min: %f Max: %f\n", getXMin(), getXMax());
	printf("Y Min: %f Max: %f\n", getYMin(), getYMax());
	printf("Height Min: %f Max: %f\n", getZMin(), getZMax());
	printf("Height Avg: %f\n", averageHeight);
	

	// close the reader

	if (laszip_close_reader(laszip_reader))
	{
		fprintf(stderr, "DLL ERROR: closing laszip reader\n");
		return false;
	}

	// destroy the reader

	if (laszip_destroy(laszip_reader))
	{
		fprintf(stderr, "DLL ERROR: destroying laszip reader\n");
		return false;
	}

	
	adjustPoints();

	setRefreshNeeded();


	return true;
}


//bool SonarPointCloud::loadLIDAR()
//{
//	printf("Loading LIDAR Point Cloud from %s\n", getName().c_str());
//
//	Renderer::getInstance().showMessage(std::string("Loading ") + getName());
//	
//	std::ifstream ifs;
//	ifs.open(getName().c_str(), std::ios::in | std::ios::binary);
//	liblas::ReaderFactory f;
//	liblas::Reader reader = f.CreateWithStream(ifs);
//	liblas::Header const& header = reader.GetHeader();
//
//	std::cout << "Compressed: " << ((header.Compressed() == true) ? "true" : "false") << std::endl;
//	std::cout << "Signature: " << header.GetFileSignature() << std::endl;
//	std::cout << "Points count: " << header.GetPointRecordsCount() << std::endl;
//	std::cout << "X Min: " << header.GetMinX() << std::endl;
//	std::cout << "X Max: " << header.GetMaxX() << std::endl;
//	std::cout << "Y Min: " << header.GetMinY() << std::endl;
//	std::cout << "Y Max: " << header.GetMaxY() << std::endl;
//	std::cout << "Z Min: " << header.GetMinZ() << std::endl;
//	std::cout << "Z Max: " << header.GetMaxZ() << std::endl;
//
//	initPoints(header.GetPointRecordsCount());
//
//	GLuint index = 0u;
//	double averageHeight = 0.0;
//
//	while (reader.ReadNextPoint())
//	{
//		liblas::Point const& p = reader.GetPoint();
//		glm::vec3 color(p.GetColor().GetRed(), p.GetColor().GetGreen(), p.GetColor().GetBlue());
//		color /= float(1 << 16);
//
//		setColoredPoint(index++, p.GetX(), p.GetY(), -p.GetZ(), color.r, color.g, color.b);
//		averageHeight += p.GetZ();
//	}
//
//	averageHeight /= m_nPoints;
//
//	printf("Loaded %d points\n", index);
//
//	printf("Original Min/Maxes:\n");
//	printf("X Min: %f Max: %f\n", getXMin(), getXMax());
//	printf("Y Min: %f Max: %f\n", getYMin(), getYMax());
//	printf("Height Min: %f Max: %f\n", getZMin(), getZMax());
//	printf("Height Avg: %f\n", averageHeight);
//	
//	ifs.close();
//
//	adjustPoints();
//
//	setRefreshNeeded();
//	
//
//	return true;
//}



bool SonarPointCloud::loadStudyCSV()
{
	printf("Loading Study Point Cloud from %s\n", getName().c_str());

	FILE *file;
	file = fopen(getName().c_str(), "r");
	if (file == NULL)
	{
		printf("ERROR reading file in %s\n", __FUNCTION__);
	}
	else
	{
		//count points
		//skip the first linesToSkip lines
		int skipped = 0;
		int tries = 0;
		char tempChar;
		while (skipped < 1 && tries < 5000) ///1=lines to skip
		{
			tries++;
			tempChar = 'a';
			while (tempChar != '\n')
			{
				tempChar = fgetc(file);
			}
			skipped++;
		}
		printf("Skipped %d characters\n", skipped);

		//now count lines of points
		double x, y, depth;
		int flag;
		unsigned int numPointsInFile = 0u;
		while (fscanf(file, "%lf,%lf,%lf,%i\n", &x, &y, &depth, &flag) != EOF)  //while another valid entry to load
			numPointsInFile++;

		initPoints(numPointsInFile);
		printf("found %d lines of points\n", numPointsInFile);

		//rewind
		rewind(file);
		//skip the first linesToSkip lines
		skipped = 0;
		tries = 0;
		while (skipped < 1 && tries < 5000) ///1=lines to skip
		{
			tries++;
			tempChar = 'a';
			while (tempChar != '\n')
			{
				tempChar = fgetc(file);
			}
			skipped++;
		}

		//now load lines of points
		GLuint index = 0u;
		double averageDepth = 0.0;
		while (fscanf(file, "%lf,%lf,%lf,%i\n", &x, &y, &depth, &flag) != EOF)  //while another valid entry to load
		{
			float tpu = flag == 1 ? 1.f : 0.f;
			setUncertaintyPoint(index++, x, y, depth, tpu, tpu);
			averageDepth += depth;
			assert(depth < 0.);
		}
		averageDepth /= m_nPoints;

		printf("Loaded %d points\n", index);

		printf("Original Min/Maxes:\n");
		printf("X Min: %f Max: %f\n", getXMin(), getXMax());
		printf("Y Min: %f Max: %f\n", getYMin(), getYMax());
		printf("Depth Min: %f Max: %f\n", getZMin(), getZMax());
		printf("Depth Avg: %f\n", averageDepth);

		fclose(file);

		adjustPoints();

		setRefreshNeeded();
	}

	return true;
}

void SonarPointCloud::update()
{
	if (m_bLoaded && (refreshNeeded || previewRefreshNeeded))
	{
		// Sub buffer data for colors...
		glNamedBufferSubData(m_glPointsBufferVBO, m_vvec3AdjustedPointsPositions.size() * sizeof(glm::vec3), m_vvec4PointsColors.size() * sizeof(glm::vec4), m_vvec4PointsColors.data());

		refreshNeeded = false;
		previewRefreshNeeded = false;
	}
}

GLuint SonarPointCloud::getVAO()
{
	return m_glVAO;
}

unsigned int SonarPointCloud::getPointCount()
{
	if (!ready())
		return 0;
	else
		return m_nPoints;
}

GLuint SonarPointCloud::getPreviewVAO()
{
	return m_glPreviewVAO;
}

unsigned int SonarPointCloud::getPreviewPointCount()
{
	return getPointCount() / m_iPreviewReductionFactor;
}

SonarPointCloud::SONAR_FILETYPE SonarPointCloud::getFiletype()
{
	return m_Sonar_Filetype;
}

bool SonarPointCloud::getRefreshNeeded()
{
	return refreshNeeded;
}

void SonarPointCloud::setRefreshNeeded()
{
	refreshNeeded = true;
	previewRefreshNeeded = true;
}


void SonarPointCloud::setColorScale(int scale)
{
	colorScale = scale;
}

int SonarPointCloud::getColorScale()
{
	return colorScale;
}


void SonarPointCloud::setColorMode(int mode)
{
	colorMode = mode;
}

int SonarPointCloud::getColorMode()
{
	return colorMode;
}

void SonarPointCloud::setColorScope(int scope)
{
	colorScope = scope;
}

int SonarPointCloud::getColorScope()
{
	return colorScope;
}

float SonarPointCloud::getMinDepthTPU()
{
	return m_fMinDepthTPU;
}
float SonarPointCloud::getMaxDepthTPU()
{
	return m_fMaxDepthTPU;
}
float SonarPointCloud::getMinPositionalTPU()
{
	return m_fMinPositionalTPU;
}
float SonarPointCloud::getMaxPositionalTPU()
{
	return m_fMaxPositionalTPU;
}

bool SonarPointCloud::s_funcDepthTPUMinCompare(SonarPointCloud* const &lhs, SonarPointCloud* const &rhs)
{
	return lhs->getMinDepthTPU() < rhs->getMinDepthTPU();
}

bool SonarPointCloud::s_funcDepthTPUMaxCompare(SonarPointCloud * const & lhs, SonarPointCloud * const & rhs)
{
	return lhs->getMaxDepthTPU() < rhs->getMaxDepthTPU();
}

bool SonarPointCloud::s_funcPosTPUMinCompare(SonarPointCloud * const & lhs, SonarPointCloud * const & rhs)
{
	return lhs->getMinPositionalTPU() < rhs->getMinPositionalTPU();
}

bool SonarPointCloud::s_funcPosTPUMaxCompare(SonarPointCloud * const & lhs, SonarPointCloud * const & rhs)
{
	return lhs->getMaxPositionalTPU() < rhs->getMaxPositionalTPU();
}

glm::vec3 SonarPointCloud::getDefaultPointColor(unsigned int index)
{
	if (m_Sonar_Filetype == SonarPointCloud::LIDAR_LAS)
	{
		return m_vvec3DefaultPointsColors[index];
	}
	else
	{
		glm::vec3 col;
		switch (m_pColorScaler->getColorMode())
		{
		case ColorScaler::Mode::ColorScale:
		{
			m_pColorScaler->getScaledColorForValue(m_vdvec3RawPointsPositions[index].z, &col.r, &col.g, &col.b);
			break;
		}
		case ColorScaler::Mode::ColorScale_BiValue:
		{
			m_pColorScaler->getBiValueScaledColor(m_vfPointsDepthTPU[index], m_vfPointsPositionTPU[index], &col.r, &col.g, &col.b);
			break;
		}
		default:
			break;
		}
		return col;
	}
}

void SonarPointCloud::adjustPoints()
{
	glm::dvec3 adjustment = getCenteringOffsets();

	for (unsigned int i = 0; i < m_nPoints; ++i)
		m_vvec3AdjustedPointsPositions[i] = m_vdvec3RawPointsPositions[i] + adjustment;
}

void SonarPointCloud::createAndLoadBuffers()
{
	m_glPointsBufferVBO = Renderer::getInstance().createInstancedDataBufferVBO(
		&m_vvec3AdjustedPointsPositions, 
		&m_vvec4PointsColors
	);

	m_glVAO = Renderer::getInstance().createInstancedPrimitiveVAO(
		"disc",
		m_glPointsBufferVBO,
		static_cast<GLsizei>(m_nPoints)
	);

	m_glPreviewVAO = Renderer::getInstance().createInstancedPrimitiveVAO(
		"disc",
		m_glPointsBufferVBO,
		static_cast<GLsizei>(m_nPoints),
		m_iPreviewReductionFactor
	);	
}

bool SonarPointCloud::ready()
{
	if (m_bLoaded)
		return true;

	if (m_Future.valid() && m_Future.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
	{
		if (m_Future.get())
		{
			printf("Successfully loaded file %s\n", getName().c_str());
			createAndLoadBuffers();
			m_bLoaded = true;
			
			Renderer::getInstance().showMessage(std::string("Successfully loaded ") + getName());

			return true;
		}
		else
			printf("ERROR: Could not load file %s\n", getName().c_str());
	}

	return false;
}

void SonarPointCloud::setEnabled(bool yesno)
{
	m_bEnabled = yesno;
}

bool SonarPointCloud::isEnabled()
{
	return m_bEnabled;
}

void SonarPointCloud::markPoint(unsigned int index, int code)
{
	m_vuiPointsMarks[index] = code;	

	glm::vec3 color;
	float a = 1.f;

	switch (code)
	{
	case 0:
		color = getDefaultPointColor(index);
		break;
	case 1:
		a = 0.f;
		break;
	case 2:
		color = glm::vec3(1.f, 0.f, 0.f);
		break;
	case 3:
		color = glm::vec3(0.f, 1.f, 0.f);
		break;
	case 4:
		color = glm::vec3(0.f, 0.f, 1.f);
		break;
	default: // if >= 100
		color = (1.f / getDefaultPointColor(index)) * (static_cast<float>(m_vuiPointsMarks[index]) - 100.f) / 100.f;
		//g = (1.f / g) * (static_cast<float>(m_vuiPointsMarks[index]) - 100.f) / 100.f;
		//b = (1.f / b) * (static_cast<float>(m_vuiPointsMarks[index]) - 100.f) / 100.f;
		a = (static_cast<float>(code) - 100.f) / 100.f;
		break;
	}

	m_vvec4PointsColors[index] = glm::vec4(color, a);

	setRefreshNeeded();
}

void SonarPointCloud::resetAllMarks()
{
	for (unsigned int i = 0; i < m_nPoints; i++)
		markPoint(i, 0);	
}

glm::vec3 SonarPointCloud::getAdjustedPointPosition(unsigned int index)
{
	return m_vvec3AdjustedPointsPositions[index];
}

glm::dvec3 SonarPointCloud::getRawPointPosition(unsigned int index)
{
	return m_vdvec3RawPointsPositions[index];
}

int SonarPointCloud::getPointMark(unsigned int index)
{
	return m_vuiPointsMarks[index];
}

float SonarPointCloud::getPointDepthTPU(unsigned int index)
{
	return m_vfPointsDepthTPU[index];
}

float SonarPointCloud::getPointPositionTPU(unsigned int index)
{
	return m_vfPointsPositionTPU[index];
}
