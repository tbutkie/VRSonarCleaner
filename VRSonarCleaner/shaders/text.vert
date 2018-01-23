layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;
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
	};

out vec2 v2TexCoords;

void main()
{
	gl_Position = m4ViewProjection * m4Model * vec4(v3Position, 1.0);
	v2TexCoords = v2TexCoordsIn;
}