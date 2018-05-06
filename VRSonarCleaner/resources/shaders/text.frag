layout(binding = DIFFUSE_TEXTURE_BINDING)
	uniform sampler2D diffuseTex;
layout(location = DIFFUSE_COLOR_UNIFORM_LOCATION)
	uniform vec4 diffColor;
layout(location = SPECULAR_COLOR_UNIFORM_LOCATION)
	uniform vec4 specColor;

in vec2 v2TexCoords;
out vec4 outputColor;

void main()
{
	vec4 sampled = vec4(1.f, 1.f, 1.f, texture(diffuseTex, v2TexCoords).r);
	
	if (sampled.a == 0.f)
		discard;

	outputColor = sampled * diffColor;
}