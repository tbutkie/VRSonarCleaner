layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;
layout(location = NORMAL_ATTRIB_LOCATION)
	in vec3 v3NormalIn;
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
		float fGlobalTime;
	};

out vec3 v3FragPos;
out vec3 v3Normal;
out vec4 v4Color;
out vec2 v2TexCoords;

float size = 0.0005f;

void main()
{
	v3FragPos = vec3(m4View * m4DataVolumeTransform * vec4(v3InstancePos, 1.f));
	v3Normal =  mat3(transpose(inverse(m4View * m4DataVolumeTransform))) * v3NormalIn;
	v4Color = v4InstanceCol;
	v2TexCoords = v2TexCoordsIn;

    gl_Position = m4Projection * (vec4(-v3Position * size, 0.f) + m4View * m4DataVolumeTransform * vec4(v3InstancePos, 1.f));
}