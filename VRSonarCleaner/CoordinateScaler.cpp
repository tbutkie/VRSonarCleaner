#include "CoordinateScaler.h"

#include <cmath>

CoordinateScaler::CoordinateScaler()
{
	//scale stuff
	lonXOrigin = 0.0;
	latYOrigin = 0.0;
	latlonscale = 1.0;
	depthScaleMode = 1;
	maxRealDepth = 5000.f;
	maxScaledDepth = 125.f;
	minLogScaleFactor1 = 1.f;
	maxLogScaleFactor1 = 500.f;
	logScaleFactor1 = 50.f;
	minLogScaleFactor2 = 1.f;
	maxLogScaleFactor2 = 30.f;
	logScaleFactor2 = 12.5f;
	linScaleFactor = 1.f;//3.28084f;//1.f;//.01f;	
	minLinScaleFactor = 0.1f;
	maxLinScaleFactor = 10.f;
}

CoordinateScaler::~CoordinateScaler()
{

}


float CoordinateScaler::getMaxScaledDepth()
{
	float maxVal = maxScaledDepth / maxRealDepth;
	return linScaleFactor / maxVal;
}
float CoordinateScaler::getLogScalingFactor()
{
	float factorRange = maxLogScaleFactor1 - minLogScaleFactor1;
	return (logScaleFactor1 - minLogScaleFactor1) / factorRange;
}
void CoordinateScaler::setMaxScaledDepth(float ofRange)//0-1 of max value
{
	//linear factor
	linScaleFactor = (maxScaledDepth * ofRange) / maxRealDepth;
	//logarithmic factors
	float scaledBottomDepth = log((maxRealDepth + logScaleFactor1) / logScaleFactor1);
	logScaleFactor2 = (linScaleFactor*maxRealDepth) / scaledBottomDepth;
}

void CoordinateScaler::setLogScaledOverallDepth(float ofRange)//0-1 of max value
{
	float factorRange = maxLogScaleFactor2 - minLogScaleFactor2;
	logScaleFactor2 = minLogScaleFactor2 + (factorRange * ofRange);
}
float CoordinateScaler::getLogScaledOverallDepth()
{
	float factorRange = maxLogScaleFactor2 - minLogScaleFactor2;
	return (logScaleFactor2 - minLogScaleFactor2) / factorRange;
}

void CoordinateScaler::setLinScaleFactor(float ofRange)
{
	float factorRange = maxLinScaleFactor - minLinScaleFactor;
	linScaleFactor = minLinScaleFactor + (factorRange * ofRange);
}

float CoordinateScaler::getLinScaleFactorOfRange()
{
	float factorRange = maxLinScaleFactor - minLinScaleFactor;
	return (linScaleFactor - minLinScaleFactor) / factorRange;
}

float CoordinateScaler::getLinScaleFactor()
{
	return linScaleFactor;
}

void CoordinateScaler::setLogScalingFactor(float ofMax)//0-1 of max value
{
	/*
	float factorRange = maxLogScaleFactor - minLogScaleFactor;
	printf("factor Range: %f\n", factorRange);
	logScaleFactor1 = factorRange * ofMax;
	*/
	ofMax *= 100;
	logScaleFactor1 = pow((ofMax / 6), (2 + (ofMax / 75))) + 10;


	printf("LSF1: %f\n", logScaleFactor1);
	float scaledBottomDepth = log((maxRealDepth + logScaleFactor1) / logScaleFactor1);
	printf("SBD: %f\n", scaledBottomDepth);
	logScaleFactor2 = (linScaleFactor*maxRealDepth) / scaledBottomDepth;
	printf("LSF2: %f\n", logScaleFactor2);
}
float CoordinateScaler::getScaledDepth(float realDepth)
{
	//realDepth = abs(realDepth);
	//return -log( (realDepth+50)/50) * 6.25;
	if (depthScaleMode == 0) //log
		return -log((realDepth + logScaleFactor1) / logScaleFactor1) * logScaleFactor2;//12.25;
	else //lin
		return -realDepth*linScaleFactor;
}
float CoordinateScaler::getUnscaledDepth(float scaledDepth)
{
	if (depthScaleMode == 0) //log
	{
		if (scaledDepth < 0)
		{
			scaledDepth = fabsf(scaledDepth);
			return -((exp(scaledDepth / logScaleFactor2) * logScaleFactor1) - logScaleFactor1);  //check this sometime
		}
		else
		{
			return (exp(scaledDepth / logScaleFactor2) * logScaleFactor1) - logScaleFactor1;  //check this sometime
		}
		//return ((pow(2.7182818284590452353, scaledDepth)/logScaleFactor2) * logScaleFactor1)-logScaleFactor1;  //check this sometime
	}
	else
	{
		if (scaledDepth < 0)
		{
			scaledDepth = fabsf(scaledDepth);
			return (scaledDepth / linScaleFactor);
		}
		else
		{
			return -(scaledDepth / linScaleFactor); //works on plane
														//return (scaledDepth/linScaleFactor); //works on flow
		}
	}


}

void CoordinateScaler::submitOriginCandidate(double lonX, double latY)
{
	printf("Old Origin %f, %f\n", lonXOrigin, latYOrigin);

	if (abs(lonX - lonXOrigin) > 50000.0)
	{
		lonXOrigin = lonX;
	}
	if (abs(latY - latYOrigin) > 50000.0)
	{
		latYOrigin = latY;
	}

	printf("New Origin %f, %f\n", lonXOrigin, latYOrigin);
}

double CoordinateScaler::getScaledLonX(double realLonX)
{
	return (realLonX - lonXOrigin) * latlonscale;
}
double CoordinateScaler::getUnscaledLonX(double scaledLonX)
{
	return (scaledLonX / latlonscale) + lonXOrigin;
}

double CoordinateScaler::getScaledLatY(double realLatY)
{
	return (realLatY - latYOrigin) * latlonscale;
}
double CoordinateScaler::getUnscaledLatY(double scaledLatY)
{
	return (scaledLatY / latlonscale) + latYOrigin;
}

float CoordinateScaler::getScaledLength(float realLength)
{
	return realLength * static_cast<float>(latlonscale);
}
float CoordinateScaler::getUnscaledLength(float scaledLength)
{
	return scaledLength / static_cast<float>(latlonscale);
}



