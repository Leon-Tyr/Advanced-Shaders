#version 330 core
out vec4 FragColor;
float LinearizeDepth(float depth) ;
vec3 setKernel();

in vec2 TexCoords;
//in vec3 fragPosCS;

uniform sampler2D scene;
uniform float near_plane;
uniform float far_plane;
uniform bool PPinverse;
void main()
{    
    //FragColor = texture(scene, TexCoords); //normal

	//float average = (FragColor.r + FragColor.g + FragColor.b) / 3.0; //greyscale
    // FragColor = vec4(average, average, average, 1.0); //greyscale

	//float depthValue = texture(scene, TexCoords).r;// depth 
	//FragColor = vec4(vec3(LinearizeDepth(depthValue) / far_plane), 1.0); // depth 

	// if statement to switch between pp colour and inverse colours
    if(PPinverse == true )
    {
		FragColor = vec4(vec3(1.0 - texture(scene, TexCoords)),1.0); //inversion
    }
    else
	 	FragColor = vec4(setKernel(), 1.0); //kernel
}  

// function for post processing depth effect 
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

// set kernel to create pp effect
vec3 setKernel()
{
	const float offset = 1.0 / 300.0; 
	vec2 offsets[9] = vec2[](
        vec2(-offset,  offset), // top-left
        vec2( 0.0f,    offset), // top-center
        vec2( offset,  offset), // top-right
        vec2(-offset,  0.0f),   // center-left
        vec2( 0.0f,    0.0f),   // center-center
        vec2( offset,  0.0f),   // center-right
        vec2(-offset, -offset), // bottom-left
        vec2( 0.0f,   -offset), // bottom-center
        vec2( offset, -offset)  // bottom-right    
    );

	 float kernel[9] = float[](
        -1, -1, -1,
        -1,  9, -1,
       -1, -1, -1
    ); //sharpen effect

    //float kernel[9] = float[](
     //1.0 / 16, 2.0 / 16, 1.0 / 16,
    //2.0 / 16, 4.0 / 16, 2.0 / 16,
    //1.0 / 16, 2.0 / 16, 1.0 / 16  
	//); //blur effect
    
	// float kernel[9] = float[](
    //    1, 1, 1,
    //   1, -8, 1,
   //      1, 1, 1
    // ); //Edge highlight effect

    vec3 sampleTex[9];
    for(int i = 0; i < 9; i++)
    {
        sampleTex[i] = vec3(texture(scene, TexCoords.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sampleTex[i] * kernel[i];

		return col;
}