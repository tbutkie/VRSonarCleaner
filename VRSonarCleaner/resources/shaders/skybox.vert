layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
	};

out vec3 v3TexCoord;

const float boxGeomConversionFactor = -99999999.f;

void main()
{
	v3TexCoord = v3Position * boxGeomConversionFactor;
    vec4 pos = m4ViewProjection * vec4(v3Position * boxGeomConversionFactor, 1.f);
    gl_Position = pos.xyww;
}