layout(location = DIFFUSE_COLOR_UNIFORM_LOCATION)
	uniform vec4 diffColor;
layout(location = SPECULAR_COLOR_UNIFORM_LOCATION)
	uniform vec4 specColor;
layout(binding = DIFFUSE_TEXTURE_BINDING)
	uniform sampler3D vectorField;
layout(binding = SPECULAR_TEXTURE_BINDING)
	uniform sampler3D vectorFieldAttribs;

in vec4 v4Color;
in vec3 v3TexCoord;
out vec4 outputColor;

void main()
{
   vec4 texColor = texture(vectorFieldAttribs, v3TexCoord);

   if (v4Color.a * diffColor.a == 0.f)
      discard;

   outputColor = vec4(1.f, 0.f, 0.f, 1.f);//v4Color * diffColor;
   outputColor.a = 0.5f;
   outputColor.a = texColor.g;
}