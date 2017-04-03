layout(binding = DIFFUSE_TEXTURE_BINDING)
	uniform sampler2D texSampler;
	
in vec2 v2TexCoord;

out vec4 outputColor;

void main()
{
   vec4 col = texture(texSampler, v2TexCoord);
   if (col.a < 0.2f)
      discard;
   outputColor = col;
}