layout(location = POSITION_ATTRIB_LOCATION)
	in vec2 v2Position;
layout(location = TEXCOORD_ATTRIB_LOCATION)
	in vec2 v2TexCoordIn;

layout(location = MVP_UNIFORM_LOCATION)
	uniform mat4 m4MVP;
	
out vec2 v2TexCoord;

void main()
{
	v2TexCoord = vec2(v2TexCoordIn.x, 1.f - v2TexCoordIn.y);
	gl_Position = m4MVP * vec4(v2Position, 0.f, 1.f);
}