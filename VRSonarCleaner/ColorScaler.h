#ifndef __ColorScaler_h__
#define __ColorScaler_h__

#include <stdio.h>
#include <math.h>


class ColorScaler
{
public:
	
	ColorScaler();
	virtual ~ColorScaler();

	void setToDefaults();
	void setColorScale(int index);
	void getScaledColor(float factor, float *r, float *g, float *b);
	void getScaledColor(float factor, float *r, float *g, float *b, int scale);
	void getOrangeBrownScaledColor(float factor, float *r, float *g, float *b);
	void getRainbowScaledColor(float factor, float *r, float *g, float *b);
	void getBandedBlueScaledColor(float factor, float *r, float *g, float *b);
	void getBandedRainbowScaledColor(float factor, float *r, float *g, float *b);
	void submitMinMaxForColorScale(float minVal, float maxVal);
	void resetMinMaxForColorScale(float minVal, float maxVal);
	float getColorScaleMin();
	float getColorScaleMax();
	float getColorScaleFactor(float depth);
	
private:
	int colorScale;
	bool colorScaleMinMaxSet;
	float minColorScaleValue, maxColorScaleValue, rangeColorScaleValue;


};

#endif
