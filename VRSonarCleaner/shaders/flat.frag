layout(location = DIFFUSE_COLOR_UNIFORM_LOCATION)
	uniform vec4 diffColor;
layout(location = SPECULAR_COLOR_UNIFORM_LOCATION)
	uniform vec4 specColor;

in vec4 v4Color;
out vec4 outputColor;

void main()
{
   outputColor = v4Color * diffColor;
}