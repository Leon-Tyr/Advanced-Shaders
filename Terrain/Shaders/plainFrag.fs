#version 330 core
float calcShadow(vec4 fragPosLightSpace);

out vec4 FragColor ;
out vec3 Color;

in vec3 gNormals ;
in vec3 gWorldPos_FS_in ;
in vec2 gTexCoords;
in float Gvisibility;
in vec4 gFragPosLightSpace;

//in vec3 normES ;
//in vec3 posES ;
//in vec2 TESTexCoords;

struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
}; 

uniform sampler2D heightMap;
uniform DirLight dirLight;
uniform vec3 viewPos ;
uniform sampler2D texture1;
uniform sampler2D shadowMap; 
uniform int shadowOn;
uniform mat4 lightSpaceMatrix;

void main()
{   
	//light space matrix for shadow calulcation  
	vec4 FragPosLightSpace = lightSpaceMatrix * vec4(gWorldPos_FS_in, 1.0);  // point as light sees it

	//used for colours
	float scale = 100;  
	float height = gWorldPos_FS_in.y/scale;

	//colours used on terrain
	vec4 green = vec4(0.039, 0.501, 0.160,0.0);
	vec4 grey = vec4(0.4,0.3,0.4,0.0);
    vec4 white = vec4(1.0,1.0,1.0,0.0);
    vec4 red = vec4(1.0,0.0,0.0,0.0);
    vec4 blue = vec4(0.144,0.660,0.780,0.0);
    vec4 lightblue = vec4(0.222,0.667,0.780,0.0);
    vec4 darkblue = vec4(0.344,0.397,0.665,0.0);
    vec4 darkgreen = vec4(0.167,0.440,0.137,0.0);
    vec4 forestgreen = vec4(0.130,0.605,0.188,0.0);
    vec4 black = vec4(0.0,0.0,0.0,0.0);

	//set colours for different heights
	if(height > 0.8)
	{
		Color = vec3(mix(grey,white,smoothstep(0.8,1.3,height)).rgb);
	}
    
	else if(height > 0.7)
	{
		Color = vec3(mix(darkgreen,grey,smoothstep(0.7,0.8,height)).rgb);
	}
    else if(height > 0.6)
	{
		Color = vec3(mix(forestgreen,darkgreen,smoothstep(0.6,0.7,height)).rgb);
	}
	else if (height > 0.5)
	{
		Color = vec3(mix(green,forestgreen,smoothstep(0.5,0.6,height)).rgb);
	}
    else if (height > 0.4)
	{
		Color = vec3(mix(forestgreen,green,smoothstep(0.4,0.5,height)).rgb);
	}
     else if (height > 0.3)
	{
		Color = vec3(mix(darkgreen,forestgreen,smoothstep(0.3,0.4,height)).rgb);
	}
     else if (height > 0.2)
	{
		Color = vec3(mix(darkblue,darkgreen,smoothstep(0.2,0.3,height)).rgb);
	}
      else if (height > 0.1)
	{
		Color = vec3(mix(lightblue,darkblue,smoothstep(0.1,0.2,height)).rgb);
	}
    else 
	{
		Color = vec3(mix(blue,lightblue,smoothstep(0.01,0.05,height)).rgb);
	}

	// blinn phong
     float shine = 0.5 ; //120000.5f ;
     vec3 viewDir = normalize(viewPos - gWorldPos_FS_in);
	 vec3 ambient = dirLight.ambient * texture(texture1, gTexCoords).rgb;   
     vec3 lightDir = normalize(-dirLight.direction);
     vec3 halfDir = normalize(lightDir - viewDir);
    // diffuse shading
    float diff = max(dot(gNormals, dirLight.direction), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-dirLight.direction, gNormals);
    float spec = pow(max(dot(halfDir, reflectDir), 0.0), shine);
    // combine results
    vec3 diffuse  = dirLight.diffuse  * diff * vec3(texture(texture1, gTexCoords));
    vec3 specular = dirLight.specular * spec * vec3(texture(texture1, gTexCoords));

	// switch on/off shadows 
	float shadow;
	if(shadowOn == 1)
		shadow = calcShadow(FragPosLightSpace);
	else
		shadow = 1;
	//apply blinn phong to frag colour
    FragColor = vec4((ambient + (1.0-shadow)*(diffuse + specular)+ Color),1.0f);
	//set fog like effect to frag colour
	vec3 sky = vec3(0.1,0.1,0.1); 
	FragColor = mix(vec4(sky, 1.0), FragColor, Gvisibility);
    }

// calculate shadows algorithm 
float calcShadow(vec4 fragPosLightSpace) 
{
    float shadow = 0.0 ; 
    // perform perspective divide values in range [-1,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // sample from shadow map  (returns a float; call it closestDepth)
	float closestDepth = texture(shadowMap, projCoords.xy).r;
    // get depth of current fragment from light's perspective ( call it current depth)
	float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float bias = 0.015;
	//if(currentDepth-bias > closestDepth)
		//shadow = 1;

		vec2 texelSize = 1.0/ textureSize(shadowMap,0);
		for(int i = -1; i <2; i++){
			for(int j = -1; j <2; j++){
				float pcf = texture(shadowMap, projCoords.xy + vec2(i,j) * texelSize).r;
				if(currentDepth - bias > pcf)
					shadow+=1;
			}
		}
				
	shadow = shadow/9;

	if(projCoords.z > 1.0f)
		shadow = 0.0f;


    return shadow;
}