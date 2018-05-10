#ifndef __ColorScaler_h__
#define __ColorScaler_h__

#include <stdio.h>
#include <math.h>


class ColorScaler
{
public:
	enum Mode {
		ColorScale,
		ColorScale_BiValue
	};

	enum ColorMap {
		OrangeBrown,
		Rainbow,
		RainbowBanded,
		BlueBanded
	};

	enum ColorMap_BiValued {
		RedBlue,
		PurpleGreen,
		Custom
	};
	
	ColorScaler();
	virtual ~ColorScaler();

	void setToDefaults();
	void setColorMap(ColorMap colorMap);
	void getScaledColorForValue(double value, float *r, float *g, float *b);
	void getScaledColor(float factor, float *r, float *g, float *b);
	void getScaledColor(float factor, float *r, float *g, float *b, ColorMap colorMapEnum);
	void submitMinMaxForColorScale(double minVal, double maxVal);
	void resetMinMaxForColorScale(double minVal, double maxVal);
	double getColorScaleMin();
	double getColorScaleMax();
	float getColorScaleFactor(double value);

	void submitBiValueScaleMinMax(double MinVal1, double MaxVal1, double MinVal2, double MaxVal2);
	void resetBiValueScaleMinMax(double MinVal1, double MaxVal1, double MinVal2, double MaxVal2);
	void setBiValueColorMap(ColorMap_BiValued biValueColorMapEnum);
	void getBiValueScaledColor(double val1, double val2, float *r, float *g, float *b);

	void setColorMode(Mode mode);
	Mode getColorMode();

private:
	void getOrangeBrownScaledColor(float factor, float *r, float *g, float *b);
	void getRainbowScaledColor(float factor, float *r, float *g, float *b);
	void getBandedBlueScaledColor(float factor, float *r, float *g, float *b);
	void getBandedRainbowScaledColor(float factor, float *r, float *g, float *b);
	
private:
	ColorMap m_ColorMap;
	bool m_bColorScaleMinMaxSet;
	double minColorScaleValue, maxColorScaleValue, rangeColorScaleValue;

	ColorMap_BiValued m_ColorMap_BiValue;
	bool m_bBiValueScaleMinMaxSet;
	double minVal1, maxVal1, rangeVal1, minVal2, maxVal2, rangeVal2;

	Mode m_ColorScaleMode;
};

#endif
