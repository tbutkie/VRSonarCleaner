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

void main()
{
	v4Color = v4ColorIn;
	gl_Position = m4ViewProjection * m4Model * vec4(v3Position, 1.0);
}