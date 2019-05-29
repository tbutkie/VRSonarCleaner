layout(location = POSITION_ATTRIB_LOCATION)
	in vec3 v3Position;
layout(location = COLOR_ATTRIB_LOCATION)
	in vec4 v4ColorIn;
	
layout(location = MODEL_MAT_UNIFORM_LOCATION)
	uniform mat4 m4Model;
	
layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
		float fGlobalTime;
	};

out vec4 v4Color;
out vec3 v3TexCoord;
const float spriteSize = 2.f;

void main()
{
	float rate = 20.f; // seconds per segment

	float easeRatio = (sin(2.f * 3.14159f * (mod(fGlobalTime, rate) / rate)) / 2.05f) + 0.5f;

	v4Color = v4ColorIn;
	v3TexCoord = v3Position + 0.5f;
	v3TexCoord.z = easeRatio;
	vec4 v4EyePos = m4View * m4Model * vec4(v3TexCoord, 1.f);

	gl_PointSize = spriteSize / length(v4EyePos.xyz);

	gl_Position = m4Projection * v4EyePos;
}