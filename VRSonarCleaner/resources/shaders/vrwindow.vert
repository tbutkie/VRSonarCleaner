layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 position;
layout(location = TEXCOORD_ATTRIB_LOCATION)
	in vec2 v2UVIn;
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
		float fGlobalTime;
	};
	
noperspective out vec2 v2UV;

void main()
{
	float aspect = v4Viewport[3] / v4Viewport[2];

	v2UV = v2UVIn;
	//v2UV.y = 0.5f + aspect * (v2UV.y - 0.5f);
	
	gl_Position = vec4(position, 1.f);
}
