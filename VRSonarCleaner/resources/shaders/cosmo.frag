layout(location = DIFFUSE_COLOR_UNIFORM_LOCATION)
	uniform vec4 diffColor; // holds cosmo attrib max vals
layout(location = SPECULAR_COLOR_UNIFORM_LOCATION)
	uniform vec4 specColor; // holds cosmo attrib min vals
layout(binding = DIFFUSE_TEXTURE_BINDING)
	uniform sampler3D vectorField; // normalized vector field
layout(binding = SPECULAR_TEXTURE_BINDING)
	uniform sampler3D vectorFieldAttribs; // raw attribs: r = velocity; g = density; b = H2II density; a = temperature
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
		float fGlobalTime;
	};

in vec4 v4Color;
in vec3 v3TexCoord;
out vec4 outputColor;

void main()
{
   vec4 texSample = texture(vectorField, v3TexCoord);
   vec4 texAttribSample = texture(vectorFieldAttribs, v3TexCoord);

   if (v4Color.a * diffColor.a == 0.f)
      discard;
	  
   float rate = 5.f; // seconds per segment
   float loopCount;
   float timeRatio = modf(fGlobalTime / rate, loopCount);

   vec4 valrange = diffColor - specColor;
   vec4 normAttribs = (texAttribSample - specColor) / valrange;

   vec4 vecColor = (texSample + 1.f) / 2.f;

   outputColor = mix(vec4(0.f, 0.f, 0.8f, 0.1f), vec4(0.8f, 0.8f, 0.f, 1.f), pow(normAttribs.g, 1.f/12.f));
   //outputColor.a = 0.5f;
   outputColor = vecColor;
   outputColor.a = (normAttribs.r);
}