#include "CoordinateScaler.h"

CoordinateScaler::CoordinateScaler()
{
	//scale stuff
	lonXOrigin = 0;
	latYOrigin = 0;
	latlonscale = 1;
	depthScaleMode = 1;
	maxRealDepth = 5000;
	maxScaledDepth = 125;
	minLogScaleFactor1 = 1;
	maxLogScaleFactor1 = 500;
	logScaleFactor1 = 50.0;
	minLogScaleFactor2 = 1;
	maxLogScaleFactor2 = 30;
	logScaleFactor2 = 12.5;
	linScaleFactor = 1;//3.28084;//1;//.01;	
	minLinScaleFactor = 0.1;
	maxLinScaleFactor = 10.0;
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
			return ((exp(scaledDepth / logScaleFactor2) * logScaleFactor1) - logScaleFactor1)*-1.0;  //check this sometime
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
			return (scaledDepth / linScaleFactor)*-1.0; //works on plane
														//return (scaledDepth/linScaleFactor); //works on flow
		}
	}


}

void CoordinateScaler::submitOriginCandidate(double lonX, double latY)
{
	printf("Old Origin %f, %f\n", lonXOrigin, latYOrigin);

	if (fabsf(lonX - lonXOrigin) > 50000)
	{
		lonXOrigin = lonX;
	}
	if (fabsf(latY - latYOrigin) > 50000)
	{
		latYOrigin = latY;
	}

	printf("New Origin %f, %f\n", lonXOrigin, latYOrigin);
}

double CoordinateScaler::getScaledLonX(double realLonX)
{
	return (realLonX - lonXOrigin)*latlonscale;
}
double CoordinateScaler::getUnscaledLonX(double scaledLonX)
{
	return (scaledLonX / latlonscale) + lonXOrigin;
}

double CoordinateScaler::getScaledLatY(double realLatY)
{
	return (realLatY - latYOrigin)*latlonscale;
}
double CoordinateScaler::getUnscaledLatY(double scaledLatY)
{
	return (scaledLatY / latlonscale) + latYOrigin;
}

float CoordinateScaler::getScaledLength(float realLength)
{
	return realLength*latlonscale;
}
float CoordinateScaler::getUnscaledLength(float scaledLength)
{
	return scaledLength / latlonscale;
}



