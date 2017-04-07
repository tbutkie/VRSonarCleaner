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
in vec3 v2TexCoords[3];
		
out vec3 GPos;
out vec3 GNorm;
out vec3 GTex;

noperspective out vec3 GEdgeDist;

vec3 getViewportCoord(vec4 ptIn)
{
	vec4 tmp = m4Projection * ptIn;

	tmp /= tmp.w;
	tmp = tmp * 0.5f + 0.5f;
	tmp[0] = tmp[0] * v4Viewport[2] + v4Viewport[0]; // scale x to VP coords
	tmp[1] = tmp[1] * v4Viewport[3] + v4Viewport[1]; // scale y to VP coords

	return vec3(tmp);
}

void main(void)
{
	// taken from 'Single-Pass Wireframe Rendering'
	vec3 p0 = getViewportCoord(gl_in[0].gl_Position);
	vec3 p1 = getViewportCoord(gl_in[1].gl_Position);
	vec3 p2 = getViewportCoord(gl_in[2].gl_Position);
	
	float a = length(p1 - p2);
	float b = length(p2 - p0);
	float c = length(p1 - p0);
	
	float alpha = acos((b*b + c*c - a*a) / (2.0*b*c));
	float beta = acos((a*a + c*c - b*b) / (2.0*a*c));
	
	float ha = abs(c * sin(beta));
	float hb = abs(c * sin(alpha));
	float hc = abs(b * sin(alpha));
	
	// Send the triangle along with the edge distances
	GEdgeDist = vec3(ha, 0, 0);
	GPos = v3FragPos[0];
	GNorm = v3Normal[0];
	GTex = v2TexCoords[0];
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	
	GEdgeDist = vec3( 0, hb, 0 );
	GPos = v3FragPos[1];
	GNorm = v3Normal[1];
	GTex = v2TexCoords[1];
	gl_Position = gl_in[1].gl_Position;
	EmitVertex();
	
	GEdgeDist = vec3( 0, 0, hc );
	GPos = v3FragPos[2];
	GNorm = v3Normal[2];
	GTex = v2TexCoords[2];
	gl_Position = gl_in[2].gl_Position;
	EmitVertex();
	
	EndPrimitive();
}