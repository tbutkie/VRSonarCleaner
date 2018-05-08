layout(location = DIFFUSE_COLOR_UNIFORM_LOCATION)
	uniform vec4 diffColor;
layout(binding = DIFFUSE_TEXTURE_BINDING)
	uniform sampler2D diffuseTex;

in vec4 v4Color;
in vec2 v2TexCoord;
out vec4 outputColor;

void main()
{
   vec4 texColor = texture(diffuseTex, v2TexCoord);

   if (texColor.a * v4Color.a * diffColor.a == 0.f)
      discard;

   outputColor = texColor * v4Color * diffColor;
}