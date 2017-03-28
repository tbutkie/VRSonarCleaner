layout(location = POSITION_ATTRIB_LOCATION)
	in vec4 position;
layout(location = NORMAL_ATTRIB_LOCATION)
	in vec3 v3NormalIn;
layout(location = TEXCOORD_ATTRIB_LOCATION)
	in vec2 v2TexCoordsIn;
	
layout(location = MVP_UNIFORM_LOCATION)
	uniform mat4 matMVP;
layout(location = MV_UNIFORM_LOCATION)
	uniform mat4 matMV;
	
out vec3 v3Normal;
out vec2 v2TexCoord;

void main()
{
	v3Normal = normalize(mat3(matMV) * v3NormalIn);
	v2TexCoord = v2TexCoordsIn;
	gl_Position = matMVP * vec4(position.xyz, 1);
}