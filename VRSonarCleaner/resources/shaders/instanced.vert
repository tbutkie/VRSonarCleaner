layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;
layout(location = COLOR_ATTRIB_LOCATION)
	in vec4 v4ColorIn;
layout(location = TEXCOORD_ATTRIB_LOCATION)
	in vec2 v2TexCoordsIn;
	
layout(location = INSTANCE_POSITION_ATTRIB_LOCATION)
	in vec3 v3InstancePos;
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
	};

out vec4 v4Color;
out vec2 v2TexCoord;

const float spriteSize = 2.f;

void main()
{
	v4Color = v4ColorIn;
	v2TexCoord = v2TexCoordsIn;
	
	mat4 m4Model = mat4(1.f);
	m4Model[3] = vec4(v3InstancePos, 1.f);

	vec4 v4EyePos = m4View * m4Model * vec4(v3Position, 1.f);

	gl_PointSize = spriteSize / length(v4EyePos.xyz);

	gl_Position = m4Projection * v4EyePos;
}