#include "ColorScaler.h"



ColorScaler::ColorScaler()
{
	setToDefaults();
}

ColorScaler::~ColorScaler()
{

}

void ColorScaler::setToDefaults()
{
	colorScale = 2;
	colorScaleMinMaxSet = false;
	minColorScaleValue = 0;
	maxColorScaleValue = 1;
	rangeColorScaleValue = 1;

	biValueScale = 0;
	biValueScaleMinMaxSet = false;
	minVal1 = 0;
	maxVal1 = 1;
	rangeVal1 = 1;
	minVal2 = 0;
	maxVal2 = 1;
	rangeVal2 = 1;
}


void ColorScaler::setColorScale(int index)
{
	colorScale = index;
}

void ColorScaler::getScaledColor(float factor, float *r, float *g, float *b, int scale)
{
	if (scale == 0)
		getOrangeBrownScaledColor(factor, r, g, b);
	else if (scale == 1)
		getBandedBlueScaledColor(factor, r, g, b);
	else  if (scale == 2)
		getRainbowScaledColor(factor, r, g, b);
	else
		getBandedRainbowScaledColor(factor, r, g, b);
}

void ColorScaler::getScaledColor(float factor, float *r, float *g, float *b)
{
	if (colorScale == 0)
		getOrangeBrownScaledColor(factor, r, g, b);
	else if (colorScale == 1)
		getBandedBlueScaledColor(factor, r, g, b);
	else if (colorScale == 2)
		getRainbowScaledColor(factor, r, g, b);
	else
		getBandedRainbowScaledColor(factor, r, g, b);

}

void ColorScaler::getScaledColorForValue(float value, float *r, float *g, float *b)
{
	getScaledColor(getColorScaleFactor(value), r, g, b);
}


void ColorScaler::getOrangeBrownScaledColor(float factor, float *r, float *g, float *b)
{
	*r = 1 - pow((factor / 1.35), 5);
	*g = 1 - pow((factor / 1.2), 3);
	*b = 1 - (factor*1.25);
	if (*b < 0)
		*b = 0;
}

void ColorScaler::getRainbowScaledColor(float factor, float *r, float *g, float *b)
{
	int sextant;
	double vsf, mid1, mid2;

	factor *= 6.0;
	sextant = (int)factor;
	vsf = factor - sextant;
	mid1 = vsf;
	mid2 = 1 - vsf;
	switch (sextant)
	{
	case 0:
		*r = 1;
		*g = 0;
		*b = mid2;
		break;
	case 1:
		*r = 1;
		*g = mid1;
		*b = 0;
		break;
	case 2:
		*r = mid2;
		*g = 1;
		*b = 0;
		break;
	case 3:
		*r = 0;
		*g = 1;
		*b = mid1;
		break;
	case 4:
		*r = 0;
		*g = mid2;
		*b = 1;
		break;
	case 5:
		*r = mid1;
		*g = 0;
		*b = 1;
		break;

	}

	return;

	
}

void ColorScaler::getBandedRainbowScaledColor(float factor, float *r, float *g, float *b)
{
	if (factor < 0.071) //violet
	{
		*r = 0.33;
		*g = 0;
		*b = 0.62;
	}
	else if (factor < 0.143)
	{
		*r = 0.18;
		*g = 0;
		*b = 0.91;
	}
	else if (factor < 0.214)
	{
		*r = 0;
		*g = 0.18;
		*b = 1;
	}
	else if (factor < 0.286) //ltblue
	{
		*r = 0;
		*g = 0.55;
		*b = 1;
	}
	else if (factor < 0.357)
	{
		*r = 0;
		*g = 0.78;
		*b = 1;
	}
	else if (factor < 0.429)
	{
		*r = 0;
		*g = 1;
		*b = 0.83; //cyan
	}
	else if (factor < 0.500) //green
	{
		*r = 0.1;
		*g = 1;
		*b = 0;
	}
	else if (factor < 0.571) //yellowish green
	{
		*r = 0.63;
		*g = 1;
		*b = 0;
	}
	else if (factor < 0.643) //very yellowish green
	{
		*r = 0.87;
		*g = 1;
		*b = 0;
	}
	else if (factor < 0.714) //very yellow
	{
		*r = 1;
		*g = 0.98;
		*b = 0;
	}
	else if (factor < 0.786) //lt orange
	{
		*r = 1;
		*g = 0.85;
		*b = 0;
	}
	else if (factor < 0.857) //orange
	{
		*r = 1;
		*g = 0.6;
		*b = 0;
	}
	else if (factor < 0.929) //dark orange
	{
		*r = 1;
		*g = 0.42;
		*b = 0;
	}
	else //red
	{
		*r = 1;
		*g = 0;
		*b = 0;
	}
	return;

}

void ColorScaler::getBandedBlueScaledColor(float factor, float *r, float *g, float *b)
{
	if (factor < 0.10)
	{
		*r = 0.01;
		*g = 0.05;
		*b = 0.20;
	}
	else if (factor < 0.20)
	{
		*r = 0.03;
		*g = 0.19;
		*b = 0.42;
	}
	else if (factor < 0.30)
	{
		*r = 0.03;
		*g = 0.32;
		*b = 0.61;
	}
	else if (factor < 0.40)
	{
		*r = 0.13;
		*g = 0.44;
		*b = 0.71;
	}
	else if (factor < 0.50)
	{
		*r = 0.26;
		*g = 0.57;
		*b = 0.78;
	}
	else if (factor < 0.60)
	{
		*r = 0.42;
		*g = 0.68;
		*b = 0.84;
	}
	else if (factor < 0.70)
	{
		*r = 0.62;
		*g = 0.79;
		*b = 0.88;
	}
	else if (factor < 0.80)
	{
		*r = 0.78;
		*g = 0.86;
		*b = 0.94;
	}
	else if (factor < 0.90)
	{
		*r = 0.87;
		*g = 0.92;
		*b = 0.97;
	}
	else
	{
		*r = 0.97;
		*g = 0.98;
		*b = 1.0;
	}
	return;
}


void ColorScaler::submitMinMaxForColorScale(float minVal, float maxVal)
{
	if (!colorScaleMinMaxSet)
	{
		minColorScaleValue = minVal;
		maxColorScaleValue = maxVal;
		colorScaleMinMaxSet = true;
	}
	else
	{
		if (minVal < minColorScaleValue)
			minColorScaleValue = minVal;
		if (maxVal > maxColorScaleValue)
			maxColorScaleValue = maxVal;
	}
	rangeColorScaleValue = maxColorScaleValue - minColorScaleValue;
	//printf("Min Max Color Depth is now: %f, %f\n", minColorScaleValue, maxColorScaleValue);
}

void ColorScaler::resetMinMaxForColorScale(float minVal, float maxVal)
{
	if (minVal < maxVal)
	{
		minColorScaleValue = minVal;
		maxColorScaleValue = maxVal;
	}
	else
	{
		minColorScaleValue = maxVal;
		maxColorScaleValue = minVal;
	}
	rangeColorScaleValue = maxColorScaleValue - minColorScaleValue;
	colorScaleMinMaxSet = true;
}

float ColorScaler::getColorScaleMin()
{
	return minColorScaleValue;
}

float ColorScaler::getColorScaleMax()
{
	return maxColorScaleValue;
}

float ColorScaler::getColorScaleFactor(float depth)
{
	if (depth <= minColorScaleValue)
		return 0;
	else if (depth >= maxColorScaleValue)
		return 1;
	else
		return ((depth - minColorScaleValue) / rangeColorScaleValue);
}

//BI VALUE SCALER:
void ColorScaler::submitBiValueScaleMinMax(float MinVal1, float MaxVal1, float MinVal2, float MaxVal2)
{
	if (!biValueScaleMinMaxSet)
	{
		minVal1 = MinVal1;
		maxVal1 = MaxVal1;
		minVal2 = MinVal2;
		maxVal2 = MaxVal2;
		colorScaleMinMaxSet = true;
	}
	else
	{
		if (MinVal1 < minVal1)
			minVal1 = MinVal1;
		if (MaxVal1 > maxVal1)
			maxVal1 = MaxVal1;
		if (MinVal2 < minVal2)
			minVal2 = MinVal2;
		if (MaxVal2 > maxVal2)
			maxVal2 = MaxVal2;
	}
	rangeVal1 = maxVal1 - minVal1;
	rangeVal2 = maxVal2 - minVal2;
	biValueScaleMinMaxSet = true;
}

void ColorScaler::resetBiValueScaleMinMax(float MinVal1, float MaxVal1, float MinVal2, float MaxVal2)
{
	if (MinVal1 < MaxVal1)
	{
		minVal1 = MinVal1;
		maxVal1 = MaxVal1;
	}
	else
	{
		minVal1 = MaxVal1;
		maxVal1 = MinVal1;
	}
	if (MinVal2 < MaxVal2)
	{
		minVal2 = MinVal2;
		maxVal2 = MaxVal2;
	}
	else
	{
		minVal2 = MaxVal2;
		maxVal2 = MinVal2;
	}
	rangeVal1 = maxVal1 - minVal1;
	rangeVal2 = maxVal2 - minVal2;
	biValueScaleMinMaxSet = true;
}


void ColorScaler::setBiValueScale(int scale)
{
	biValueScale = scale;
}

void ColorScaler::getBiValueScaledColor(float val1, float val2, float *r, float *g, float *b)
{
	float factor1, factor2;
	
	if (val1 <= minVal1)
		factor1 = 0;
	else if (val1 >= maxVal1)
		factor1 = 1;
	else
		factor1 = ((val1 - minVal1) / rangeVal1);
	
	if (val2 <= minVal2)
		factor2 = 0;
	else if (val2 >= maxVal2)
		factor2 = 1;
	else
		factor2 = ((val2 - minVal2) / rangeVal2);

	if (biValueScale == 0)
	{
		*r = val1;
		*g = 0;
		*b = val2;
	}
	else if (biValueScale == 1) //custom error
	{
		if (factor1 < 0.333)
		{
			if (factor2 < 0.333)
			{
				*r = 1.0;
				*g = 1.0;
				*b = 1.0;
			}
			else if (factor2 < 0.666)
			{
				*r = 1.0;
				*g = 1.0;
				*b = 0.75;
			}
			else
			{
				*r = 1.0;
				*g = 0.75;
				*b = 0.5;
			}
		}
		else if (factor1 < 0.666)
		{
			if (factor2 < 0.333)
			{
				*r = 1.0;
				*g = 0.85;
				*b = 0.15;
			}
			else if (factor2 < 0.666)
			{
				*r = 1.0;
				*g = 0.75;
				*b = 0.25;
			}
			else
			{
				*r = 1.0;
				*g = 0.50;
				*b = 0.40;
			}
		}
		else
		{
			if (factor2 < 0.333)
			{
				*r = 1.0;
				*g = 0.50;
				*b = 0.50;
			}
			else if (factor2 < 0.666)
			{
				*r = 1.0;
				*g = 0.35;
				*b = 0.35;
			}
			else
			{
				*r = 1.0;
				*g = 0.15;
				*b = 0.15;
			}
		}
	}
	else if (biValueScale == 2) //purple/green bimap
	{
		if (factor1 < 0.333)
		{
			if (factor2 < 0.333)
			{
				*r = 0.95;
				*g = 0.95;
				*b = 0.95;
			}
			else if (factor2 < 0.666)
			{
				*r = 0.76;
				*g = 0.94;
				*b = 0.81;
			}
			else
			{
				*r = 0.54;
				*g = 0.88;
				*b = 0.68;
			}
		}
		else if (factor1 < 0.666)
		{
			if (factor2 < 0.333)
			{
				*r = 0.92;
				*g = 0.77;
				*b = 0.87;
			}
			else if (factor2 < 0.666)
			{
				*r = 0.62;
				*g = 0.77;
				*b = 0.83;
			}
			else
			{
				*r = 0.49;
				*g = 0.77;
				*b = 0.69;
			}
		}
		else
		{
			if (factor2 < 0.333)
			{
				*r = 0.90;
				*g = 0.64;
				*b = 0.82;
			}
			else if (factor2 < 0.666)
			{
				*r = 0.73;
				*g = 0.62;
				*b = 0.81;
			}
			else
			{
				*r = 0.48;
				*g = 0.57;
				*b = 0.68;
			}
		}
	}


}

