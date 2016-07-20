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
	void getScaledColorForValue(float value, float *r, float *g, float *b);
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
	float getColorScaleFactor(float value);

	void submitBiValueScaleMinMax(float MinVal1, float MaxVal1, float MinVal2, float MaxVal2);
	void resetBiValueScaleMinMax(float MinVal1, float MaxVal1, float MinVal2, float MaxVal2);
	void setBiValueScale(int scale);
	void getBiValueScaledColor(float val1, float val2, float *r, float *g, float *b);
	
private:
	int colorScale;
	bool colorScaleMinMaxSet;
	float minColorScaleValue, maxColorScaleValue, rangeColorScaleValue;

	int biValueScale;
	bool biValueScaleMinMaxSet;
	float minVal1, maxVal1, rangeVal1, minVal2, maxVal2, rangeVal2;

};

#endif
