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
	bool landscape = v4Viewport[2] > v4Viewport[3] ? true : false;
	float aspect = landscape ? v4Viewport[2] / v4Viewport[3] : v4Viewport[3] / v4Viewport[2];
		
	switch(gl_VertexID)
	{
	case 0:
		if (landscape)
			v2UV = vec2(0.f, 0.f);
		else
			v2UV = vec2(0.f, 0.5f - 0.5f * aspect);
		break;
	case 1:
		if (landscape)
			v2UV = vec2(1.f, 0.f);
		else
			v2UV = vec2(1.f, 0.5f - 0.5f * aspect);
		break;
	case 2:
		if (landscape)
			v2UV = vec2(0.f, 1.f);
		else
			v2UV = vec2(0.f, 0.5f + 0.5f * aspect);
		break;
	case 3:
		if (landscape)
			v2UV = vec2(1.f, 1.f);	
		else
			v2UV = vec2(1.f, 0.5f + 0.5f * aspect);	
		break;
	};
	
	gl_Position = position;
}
