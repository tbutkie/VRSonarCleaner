layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(std140, binding = SCENE_UNIFORM_BUFFER_LOCATION) 
	uniform FrameUniforms
	{
		vec4 v4Viewport;
		mat4 m4View;
		mat4 m4Projection;
		mat4 m4ViewProjection;
	};
	
in vec3 v3FragPos[3];
in vec3 v3Normal[3];
in vec2 v2TexCoords[3];
		
out vec3 GPos;
out vec3 GNorm;
out vec2 GTex;

noperspective out vec3 GEdgeDist;

void main(void)
{
	vec2 WIN_SCALE = vec2(v4Viewport[2] - v4Viewport[0], v4Viewport[3] - v4Viewport[1]);
	
	vec2 p0 = WIN_SCALE * gl_in[0].gl_Position.xy/gl_in[0].gl_Position.w;
	vec2 p1 = WIN_SCALE * gl_in[1].gl_Position.xy/gl_in[1].gl_Position.w;
	vec2 p2 = WIN_SCALE * gl_in[2].gl_Position.xy/gl_in[2].gl_Position.w;
	vec2 v0 = p2-p1;
	vec2 v1 = p2-p0;
	vec2 v2 = p1-p0;
	float area = abs(v1.x*v2.y - v1.y * v2.x);

	GEdgeDist = vec3(area/length(v0),0,0);
	GPos = v3FragPos[0];
	GNorm = v3Normal[0];
	GTex = v2TexCoords[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	
	GEdgeDist = vec3(0,area/length(v1),0);
	GPos = v3FragPos[1];
	GNorm = v3Normal[1];
	GTex = v2TexCoords[2];
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	
	GEdgeDist = vec3(0,0,area/length(v2));
	GPos = v3FragPos[2];
	GNorm = v3Normal[2];
	GTex = v2TexCoords[2];
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	
	EndPrimitive();
}