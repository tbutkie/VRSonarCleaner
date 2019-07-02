layout(location = DIFFUSE_COLOR_UNIFORM_LOCATION)
	uniform vec4 diffColor;
layout(location = SPECULAR_COLOR_UNIFORM_LOCATION)
	uniform vec4 specColor;

in vec2 v2TexCoords;
out vec4 outputColor;

float grid(vec2 st, float res)
{
  vec2 grid = fract(st*res);
  return (step(res,grid.x) * step(res,grid.y));
}

void main()
{
	float gridRes = 20.f;
	float thickness = 0.85f; // as a percent of the grid cell size

	float xGrid = mod(v2TexCoords.x, 1.f / gridRes) / ( 1.f / gridRes);
	float yGrid = mod(v2TexCoords.y, 1.f / gridRes) / ( 1.f / gridRes);

	//if ((xGrid <= 0.5f - thickness / 2.f || xGrid >= 0.5f + thickness / 2.f) ||
	//	(yGrid <= 0.5f - thickness / 2.f || yGrid >= 0.5f + thickness / 2.f ))
	//	discard;

	outputColor = vec4(1.f, 1.f, 0.f, 0.25f);

	if ((xGrid <= 0.5f - thickness / 2.f || xGrid >= 0.5f + thickness / 2.f) ||
		(yGrid <= 0.5f - thickness / 2.f || yGrid >= 0.5f + thickness / 2.f ))
		outputColor = vec4(0.f, 1.f, 1.f, 0.f);	  
}