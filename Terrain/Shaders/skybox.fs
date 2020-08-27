#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
	//set textures for each image for skybox 
    FragColor = texture(skybox, TexCoords);
}