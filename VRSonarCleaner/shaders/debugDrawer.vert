layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;
layout(location = COLOR_ATTRIB_LOCATION)
	in vec4 v4ColorIn;

layout(location = MVP_UNIFORM_LOCATION)
	uniform mat4 matVP;

out vec4 v4Color;

void main()
{
	v4Color = v4ColorIn;
	gl_Position = matVP * vec4(v3Position, 1.0);
}