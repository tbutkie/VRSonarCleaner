layout(location = POSITION_ATTRIB_LOCATION)
	in vec4 position;
layout(location = TEXCOORD_ATTRIB_LOCATION)
	in vec2 v2UVIn;

noperspective out vec2 v2UV;

void main()
{
	v2UV = v2UVIn;
	gl_Position = position;
}
