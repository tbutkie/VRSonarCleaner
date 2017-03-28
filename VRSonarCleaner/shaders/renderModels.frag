layout(location = LIGHTDIR_UNIFORM_LOCATION)
	uniform vec3 lightDir;
	
layout(binding = DIFFUSE_TEXTURE_BINDING)
	uniform sampler2D diffuse;
	
in vec3 v3Normal;
in vec2 v2TexCoord;

out vec4 outputColor;

void main()
{
   outputColor = max(dot(v3Normal, lightDir), 0.0) * texture( diffuse, v2TexCoord);
}