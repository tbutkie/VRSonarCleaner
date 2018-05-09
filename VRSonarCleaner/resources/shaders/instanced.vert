layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;
layout(location = COLOR_ATTRIB_LOCATION)
	in vec4 v4ColorIn;
layout(location = TEXCOORD_ATTRIB_LOCATION)
	in vec2 v2TexCoordsIn;
	
layout(location = INSTANCE_POSITION_ATTRIB_LOCATION)
	in vec3 v3InstancePos;
layout(location = INSTANCE_COLOR_ATTRIB_LOCATION)
	in vec4 v4InstanceCol;
	
layout(location = MODEL_MAT_UNIFORM_LOCATION)
	uniform mat4 m4DataVolumeTransform;
	
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

float size = 0.0025f;

void main()
{
	v4Color = v4InstanceCol;
	v2TexCoord = v2TexCoordsIn;
	
    gl_Position = m4Projection * (vec4(v3Position * size, 1.f) + m4View * m4DataVolumeTransform * vec4(v3InstancePos, 1.f));
}