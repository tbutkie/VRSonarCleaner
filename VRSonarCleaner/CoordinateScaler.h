#ifndef __CoordinateScaler_h__
#define __CoordinateScaler_h__

#include <math.h>
#include <stdio.h>

class CoordinateScaler
{
public:
	CoordinateScaler();
	virtual ~CoordinateScaler();

	void submitOriginCandidate(double lonX, double latY);
	float getMaxScaledDepth();
	float getLogScalingFactor();
	float getLinScaleFactor();
	float getLinScaleFactorOfRange();
	void setLinScaleFactor(float ofRange);
	void setMaxScaledDepth(float ofRange);
	void setLogScaledOverallDepth(float ofRange);
	float getLogScaledOverallDepth();
	void setLogScalingFactor(float ofMax);
	float getScaledDepth(float realDepth);
	float getUnscaledDepth(float scaledDepth);

	double getScaledLonX(double realLonX);
	double getUnscaledLonX(double scaledLonX);

	double getScaledLatY(double realLatY);
	double getUnscaledLatY(double scaledLatY);
	float getScaledLength(float realLength);
	float getUnscaledLength(float scaledLength); //for non origin'ed lengths such as a radius of a circle


private:

	double latlonscale;
	double lonXOrigin, latYOrigin; //use to remove precision issues with large coordinates like UTM
	int depthScaleMode;
	float maxRealDepth;
	float maxScaledDepth;
	float minLogScaleFactor1;
	float maxLogScaleFactor1;
	float logScaleFactor1;
	float minLogScaleFactor2;
	float maxLogScaleFactor2;
	float logScaleFactor2;
	float linScaleFactor;
	float minLinScaleFactor;
	float maxLinScaleFactor;


};

#endif
