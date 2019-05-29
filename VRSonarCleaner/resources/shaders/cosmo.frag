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

vec4 sharpen(in sampler3D tex, in vec3 coords, in vec3 renderSize) {
  float dx = 1.0 / renderSize.x;
  float dy = 1.0 / renderSize.y;
  float dz = 0.0 / renderSize.z;

  vec4 sum = vec4(0.0);
  
  // z layer before
  sum += -1. * texture(tex, coords + vec3( -1.0 * dx , 0.0 * dy, -1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 0.0 * dx , -1.0 * dy, -1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( -1.0 * dx , -1.0 * dy, -1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 0.0 * dx , 0.0 * dy, -1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 0.0 * dx , 1.0 * dy, -1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 1.0 * dx , 0.0 * dy, -1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 1.0 * dx , 1.0 * dy, -1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 1.0 * dx , -1.0 * dy, -1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( -1.0 * dx , 1.0 * dy, -1.0 * dz));

  // z layer at
  sum += -1. * texture(tex, coords + vec3( -1.0 * dx , 0.0 * dy, 0.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 0.0 * dx , -1.0 * dy, 0.0 * dz));
  sum += -1. * texture(tex, coords + vec3( -1.0 * dx , -1.0 * dy, 0.0 * dz));
  sum += 27. * texture(tex, coords + vec3( 0.0 * dx , 0.0 * dy, 0.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 0.0 * dx , 1.0 * dy, 0.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 1.0 * dx , 0.0 * dy, 0.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 1.0 * dx , 1.0 * dy, 0.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 1.0 * dx , -1.0 * dy, 0.0 * dz));
  sum += -1. * texture(tex, coords + vec3( -1.0 * dx , 1.0 * dy, 0.0 * dz));

  
  // z layer after
  sum += -1. * texture(tex, coords + vec3( -1.0 * dx , 0.0 * dy, 1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 0.0 * dx , -1.0 * dy, 1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( -1.0 * dx , -1.0 * dy, 1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 0.0 * dx , 0.0 * dy, 1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 0.0 * dx , 1.0 * dy, 1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 1.0 * dx , 0.0 * dy, 1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 1.0 * dx , 1.0 * dy, 1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( 1.0 * dx , -1.0 * dy, 1.0 * dz));
  sum += -1. * texture(tex, coords + vec3( -1.0 * dx , 1.0 * dy, 1.0 * dz));
  return sum;
}

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

	//outputColor = mix(vec4(0.f, 0.f, 0.8f, 1.f), vec4(0.8f, 0.8f, 0.f, 1.f), pow(normAttribs.g, 1.f/12.f));
	//outputColor.a = 0.5f;
	//outputColor = vecColor;
	//outputColor.a = (normAttribs.r);

	vec4 sharpSample = sharpen(vectorFieldAttribs, v3TexCoord, vec3(400, 400, 400));
	vec4 sharpColor = (sharpSample + 1.f) / 2.f;
	//outputColor.rgb = vec3(sharpSample.g);
	//outputColor.a = 1.f;
	outputColor = vecColor;

	outputColor.a = sharpSample.g > 0.f ? sharpSample.g : 0.f;

	// color areas with no data red
	if (texAttribSample.r + texAttribSample.g + texAttribSample.b + texAttribSample.a == 0.f) 
		outputColor = vec4(1.f, 0.f, 0.f, 1.f);
}