layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;
layout(location = NORMAL_ATTRIB_LOCATION)
	in vec3 v3NormalIn;
layout(location = TEXCOORD_ATTRIB_LOCATION)
	in vec2 v2TexCoordsIn;
	
layout(location = MVP_UNIFORM_LOCATION)
	uniform mat4 m4MVP;
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4MV;
		mat4 m4MVInvTrans;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
	};

out vec3 v3Normal;
out vec3 v3FragPos;
out vec2 v2TexCoords;

void main()
{
	gl_Position = m4MVP * vec4(v3Position, 1.f);
	v3FragPos = vec3(m4MV * vec4(v3Position, 1.f));
	v3Normal =  mat3(m4MVInvTrans) * v3NormalIn;
	v2TexCoords = v2TexCoordsIn;
}