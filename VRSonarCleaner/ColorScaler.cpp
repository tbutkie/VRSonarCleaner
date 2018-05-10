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
	m_ColorScaleMode = Mode::ColorScale;

	m_ColorMap = Rainbow;
	m_bColorScaleMinMaxSet = false;
	minColorScaleValue = 0.f;
	maxColorScaleValue = 1.f;
	rangeColorScaleValue = 1.f;

	m_ColorMap_BiValue = RedBlue;
	m_bBiValueScaleMinMaxSet = false;
	minVal1 = 0.f;
	maxVal1 = 1.f;
	rangeVal1 = 1.f;
	minVal2 = 0.f;
	maxVal2 = 1.f;
	rangeVal2 = 1.f;
}


void ColorScaler::setColorMap(ColorMap colorMap)
{
	m_ColorMap = colorMap;
}

void ColorScaler::getScaledColor(float factor, float *r, float *g, float *b, ColorMap colorMapEnum)
{
	switch (colorMapEnum) {
	case OrangeBrown:
		getOrangeBrownScaledColor(factor, r, g, b);
		break;
	case BlueBanded:
		getBandedBlueScaledColor(factor, r, g, b);
		break;
	case Rainbow:
		getRainbowScaledColor(factor, r, g, b);
		break;
	case RainbowBanded:
		getBandedRainbowScaledColor(factor, r, g, b);
		break;
	default:
		printf("Error: ColorMap %d\n", colorMapEnum);
		break;
	}
}

void ColorScaler::getScaledColor(float factor, float *r, float *g, float *b)
{
	switch (m_ColorMap) {
	case OrangeBrown:
		getOrangeBrownScaledColor(factor, r, g, b);
		break;
	case BlueBanded:
		getBandedBlueScaledColor(factor, r, g, b);
		break;
	case Rainbow:
		getRainbowScaledColor(factor, r, g, b);
		break;
	case RainbowBanded:
		getBandedRainbowScaledColor(factor, r, g, b);
		break;
	default:
		break;
	}
}

void ColorScaler::getScaledColorForValue(double value, float *r, float *g, float *b)
{
	getScaledColor(getColorScaleFactor(value), r, g, b);
}


void ColorScaler::getOrangeBrownScaledColor(float factor, float *r, float *g, float *b)
{
	*r = 1.f - powf((factor / 1.35f), 5.f);
	*g = 1.f - powf((factor / 1.2f), 3.f);
	*b = 1.f - (factor*1.25f);
	if (*b < 0.f)
		*b = 0.f;
}

void ColorScaler::getRainbowScaledColor(float factor, float *r, float *g, float *b)
{
	int sextant;
	float vsf, mid1, mid2;

	factor *= 6.f;
	sextant = (int)factor;
	vsf = factor - floorf(factor);
	mid1 = vsf;
	mid2 = 1.f - vsf;
	switch (sextant)
	{
	case 0:
		*r = 1.f;
		*g = 0.f;
		*b = mid2;
		break;
	case 1:
		*r = 1.f;
		*g = mid1;
		*b = 0.f;
		break;
	case 2:
		*r = mid2;
		*g = 1.f;
		*b = 0.f;
		break;
	case 3:
		*r = 0.f;
		*g = 1.f;
		*b = mid1;
		break;
	case 4:
		*r = 0.f;
		*g = mid2;
		*b = 1.f;
		break;
	case 5:
		*r = mid1;
		*g = 0.f;
		*b = 1.f;
		break;

	}

	return;

	
}

void ColorScaler::getBandedRainbowScaledColor(float factor, float *r, float *g, float *b)
{
	if (factor < 0.071f) //violet
	{
		*r = 0.33f;
		*g = 0.f;
		*b = 0.62f;
	}
	else if (factor < 0.143f)
	{
		*r = 0.18f;
		*g = 0.f;
		*b = 0.91f;
	}
	else if (factor < 0.214f)
	{
		*r = 0.f;
		*g = 0.18f;
		*b = 1.f;
	}
	else if (factor < 0.286f) //ltblue
	{
		*r = 0.f;
		*g = 0.55f;
		*b = 1.f;
	}
	else if (factor < 0.357f)
	{
		*r = 0.f;
		*g = 0.78f;
		*b = 1.f;
	}
	else if (factor < 0.429f)
	{
		*r = 0.f;
		*g = 1.f;
		*b = 0.83f; //cyan
	}
	else if (factor < 0.5f) //green
	{
		*r = 0.1f;
		*g = 1.f;
		*b = 0.f;
	}
	else if (factor < 0.571f) //yellowish green
	{
		*r = 0.63f;
		*g = 1.f;
		*b = 0.f;
	}
	else if (factor < 0.643f) //very yellowish green
	{
		*r = 0.87f;
		*g = 1.f;
		*b = 0.f;
	}
	else if (factor < 0.714f) //very yellow
	{
		*r = 1.f;
		*g = 0.98f;
		*b = 0.f;
	}
	else if (factor < 0.786f) //lt orange
	{
		*r = 1;
		*g = 0.85f;
		*b = 0.f;
	}
	else if (factor < 0.857f) //orange
	{
		*r = 1.f;
		*g = 0.6f;
		*b = 0.f;
	}
	else if (factor < 0.929f) //dark orange
	{
		*r = 1.f;
		*g = 0.42f;
		*b = 0.f;
	}
	else //red
	{
		*r = 1.f;
		*g = 0.f;
		*b = 0.f;
	}
	return;

}

void ColorScaler::getBandedBlueScaledColor(float factor, float *r, float *g, float *b)
{
	if (factor < 0.1f)
	{
		*r = 0.01f;
		*g = 0.05f;
		*b = 0.20f;
	}
	else if (factor < 0.2f)
	{
		*r = 0.03f;
		*g = 0.19f;
		*b = 0.42f;
	}
	else if (factor < 0.3f)
	{
		*r = 0.03f;
		*g = 0.32f;
		*b = 0.61f;
	}
	else if (factor < 0.4f)
	{
		*r = 0.13f;
		*g = 0.44f;
		*b = 0.71f;
	}
	else if (factor < 0.5f)
	{
		*r = 0.26f;
		*g = 0.57f;
		*b = 0.78f;
	}
	else if (factor < 0.6f)
	{
		*r = 0.42f;
		*g = 0.68f;
		*b = 0.84f;
	}
	else if (factor < 0.7f)
	{
		*r = 0.62f;
		*g = 0.79f;
		*b = 0.88f;
	}
	else if (factor < 0.8f)
	{
		*r = 0.78f;
		*g = 0.86f;
		*b = 0.94f;
	}
	else if (factor < 0.9f)
	{
		*r = 0.87f;
		*g = 0.92f;
		*b = 0.97f;
	}
	else
	{
		*r = 0.97f;
		*g = 0.98f;
		*b = 1.f;
	}
	return;
}


void ColorScaler::submitMinMaxForColorScale(double minVal, double maxVal)
{
	if (!m_bColorScaleMinMaxSet)
	{
		minColorScaleValue = minVal;
		maxColorScaleValue = maxVal;
		m_bColorScaleMinMaxSet = true;
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

void ColorScaler::resetMinMaxForColorScale(double minVal, double maxVal)
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
	m_bColorScaleMinMaxSet = true;
}

double ColorScaler::getColorScaleMin()
{
	return minColorScaleValue;
}

double ColorScaler::getColorScaleMax()
{
	return maxColorScaleValue;
}

float ColorScaler::getColorScaleFactor(double value)
{
	if (value <= minColorScaleValue)
		return 0.f;
	else if (value >= maxColorScaleValue)
		return 1.f;
	else
		return static_cast<float>((value - minColorScaleValue) / rangeColorScaleValue);
}

//BI VALUE SCALER:
void ColorScaler::submitBiValueScaleMinMax(double MinVal1, double MaxVal1, double MinVal2, double MaxVal2)
{
	if (!m_bBiValueScaleMinMaxSet)
	{
		minVal1 = MinVal1;
		maxVal1 = MaxVal1;
		minVal2 = MinVal2;
		maxVal2 = MaxVal2;
		m_bBiValueScaleMinMaxSet = true;
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
	m_bBiValueScaleMinMaxSet = true;
}

void ColorScaler::resetBiValueScaleMinMax(double MinVal1, double MaxVal1, double MinVal2, double MaxVal2)
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
	m_bBiValueScaleMinMaxSet = true;
}


void ColorScaler::setBiValueColorMap(ColorMap_BiValued biValueColorMapEnum)
{
	m_ColorMap_BiValue = biValueColorMapEnum;
}

void ColorScaler::getBiValueScaledColor(double val1, double val2, float *r, float *g, float *b)
{
	float factor1, factor2;
	
	if (val1 <= minVal1)
		factor1 = 0.f;
	else if (val1 >= maxVal1)
		factor1 = 1.f;
	else
		factor1 = static_cast<float>((val1 - minVal1) / rangeVal1);
	
	if (val2 <= minVal2)
		factor2 = 0.f;
	else if (val2 >= maxVal2)
		factor2 = 1.f;
	else
		factor2 = static_cast<float>((val2 - minVal2) / rangeVal2);

	switch (m_ColorMap_BiValue)
	{
	case RedBlue:
	{
		*r = static_cast<float>(val1);
		*g = 0.f;
		*b = static_cast<float>(val2);
		break;
	}
	case PurpleGreen:
	{
		if (factor1 < 0.333f)
		{
			if (factor2 < 0.333f)
			{
				*r = 0.95f;
				*g = 0.95f;
				*b = 0.95f;
			}
			else if (factor2 < 0.666f)
			{
				*r = 0.76f;
				*g = 0.94f;
				*b = 0.81f;
			}
			else
			{
				*r = 0.54f;
				*g = 0.88f;
				*b = 0.68f;
			}
		}
		else if (factor1 < 0.666f)
		{
			if (factor2 < 0.333f)
			{
				*r = 0.92f;
				*g = 0.77f;
				*b = 0.87f;
			}
			else if (factor2 < 0.666f)
			{
				*r = 0.62f;
				*g = 0.77f;
				*b = 0.83f;
			}
			else
			{
				*r = 0.49f;
				*g = 0.77f;
				*b = 0.69f;
			}
		}
		else
		{
			if (factor2 < 0.333f)
			{
				*r = 0.9f;
				*g = 0.64f;
				*b = 0.82f;
			}
			else if (factor2 < 0.666f)
			{
				*r = 0.73f;
				*g = 0.62f;
				*b = 0.81f;
			}
			else
			{
				*r = 0.48f;
				*g = 0.57f;
				*b = 0.68f;
			}
		}
		break;
	}
	case Custom:
	{
		if (factor1 < 0.333f)
		{
			if (factor2 < 0.333f)
			{
				*r = 1.f;
				*g = 1.f;
				*b = 1.f;
			}
			else if (factor2 < 0.666f)
			{
				*r = 1.f;
				*g = 1.f;
				*b = 0.65f;
			}
			else
			{
				*r = 1.f;
				*g = 0.75f;
				*b = 0.45f;
			}
		}
		else if (factor1 < 0.666f)
		{
			if (factor2 < 0.333f)
			{
				*r = 1.f;
				*g = 0.85f;
				*b = 0.15f;
			}
			else if (factor2 < 0.666f)
			{
				*r = 1.f;
				*g = 0.75f;
				*b = 0.25f;
			}
			else
			{
				*r = 1.f;
				*g = 0.5f;
				*b = 0.4f;
			}
		}
		else
		{
			if (factor2 < 0.333f)
			{
				*r = 1.f;
				*g = 0.5f;
				*b = 0.5f;
			}
			else if (factor2 < 0.666f)
			{
				*r = 1.f;
				*g = 0.35f;
				*b = 0.35f;
			}
			else
			{
				*r = 1.f;
				*g = 0.15f;
				*b = 0.15f;
			}
		}
		break;
	}
	default:
		break;
	}
}

void ColorScaler::setColorMode(Mode mode)
{
	m_ColorScaleMode = mode;
}

ColorScaler::Mode ColorScaler::getColorMode()
{
	return m_ColorScaleMode;
}

