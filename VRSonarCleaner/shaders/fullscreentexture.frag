layout(location = DIFFUSE_TEXTURE_BINDING)
	uniform sampler2D mytexture;

noperspective in vec2 v2UV;
out vec4 outputColor;

void main()
{
	outputColor = texture(mytexture, v2UV);
}
