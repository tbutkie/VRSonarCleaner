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
		//              	// base alignment  // aligned offset
		mat4 m4MV;     		// 16              // 0   (column 0)
							// 16              // 16  (column 1)
							// 16              // 32  (column 2)
							// 16              // 48  (column 3)
		mat4 m4MVInvTrans;	// 16              // 64  (column 0)
							// 16              // 80  (column 1)
							// 16              // 96  (column 2)
							// 16              // 112 (column 3)
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