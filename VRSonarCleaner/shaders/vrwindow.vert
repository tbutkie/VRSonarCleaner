layout(location = POSITION_ATTRIB_LOCATION)
	in vec4 position;
layout(location = TEXCOORD_ATTRIB_LOCATION)
	in vec2 v2UVIn;
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
	};
	
noperspective out vec2 v2UV;

void main()
{
	float aspect = v4Viewport[3] / v4Viewport[2];
		
	switch(gl_VertexID)
	{
	case 0:
		v2UV = vec2(0.f, 0.5f - 0.5f * aspect);
		break;
	case 1:
		v2UV = vec2(1.f, 0.5f - 0.5f * aspect);
		break;
	case 2:
		v2UV = vec2(0.f, 0.5f + 0.5f * aspect);
		break;
	case 3:
		v2UV = vec2(1.f, 0.5f + 0.5f * aspect);	
		break;
	};
	
	gl_Position = position;
}
