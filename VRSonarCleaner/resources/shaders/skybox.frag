out vec4 color;

in vec3 v3TexCoord;

uniform samplerCube skybox;

void main()
{    
    color = texture(skybox, v3TexCoord);
}