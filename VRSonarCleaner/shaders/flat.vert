layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;
layout(location = COLOR_ATTRIB_LOCATION)
	in vec4 v4ColorIn;
	
layout(location = MODEL_MAT_UNIFORM_LOCATION)
	uniform mat4 m4Model;
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
	};

out vec4 v4Color;

float spriteSize = 50.f;

const float minPointScale = 0.1f;
const float maxPointScale = 1.f;
const float maxDistance   = 1.f;

void main()
{
	v4Color = v4ColorIn;
	
	float cameraDist = distance(v3Position, -(m4View[3].xyz));
    float pointScale = 1.f - (cameraDist / maxDistance);
	pointScale = clamp(minPointScale, maxPointScale, pointScale);

    // Set GL globals and forward the color:
    gl_PointSize = spriteSize * pointScale;

	gl_Position = m4ViewProjection * m4Model * vec4(v3Position, 1.f);
}