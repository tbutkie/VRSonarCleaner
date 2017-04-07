layout(location = POSITION_ATTRIB_LOCATION)
	in vec2 v2Position;
layout(location = TEXCOORD_ATTRIB_LOCATION)
	in vec2 v2TexCoordIn;

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
	
out vec2 v2TexCoord;

void main()
{
	v2TexCoord = vec2(v2TexCoordIn.x, 1.f - v2TexCoordIn.y);
	gl_Position = m4ViewProjection * m4Model * vec4(v2Position, 0.f, 1.f);
}