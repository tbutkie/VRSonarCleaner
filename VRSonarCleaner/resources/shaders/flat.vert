layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;
layout(location = COLOR_ATTRIB_LOCATION)
	in vec4 v4ColorIn;
layout(location = TEXCOORD_ATTRIB_LOCATION)
	in vec2 v2TexCoordsIn;
	
layout(location = MODEL_MAT_UNIFORM_LOCATION)
	uniform mat4 m4Model;
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
		float fGlobalTime;
	};

out vec4 v4Color;
out vec2 v2TexCoords;

const float spriteSize = 2.f;

void main()
{
	v2TexCoords = v2TexCoordsIn;

	v4Color = v4ColorIn;
	
	vec4 v4EyePos = m4View * m4Model * vec4(v3Position, 1.f);

	gl_PointSize = spriteSize / length(v4EyePos.xyz);

	gl_Position = m4Projection * v4EyePos;
}