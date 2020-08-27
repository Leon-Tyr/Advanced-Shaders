#version 330 core
out vec4 FragColor;
float calcShadow(vec4 fragPosLightSpace);

in vec2 TexCoords;
in vec3 Normals;
in vec3 FragPos;
in vec4 FragPosLightSpace;

struct DirLight {
    vec3 direction;  // direction of light
    vec3 ambient;    // ambient , spsc, and diffuse values
    vec3 diffuse;
    vec3 specular;
}; 

uniform sampler2D texture1;
uniform sampler2D shadowMap;   // shadow map texture
uniform DirLight dirLight;
uniform vec3 viewPos ;
uniform int shadowOn;

void main()
{             
	 float shine = 0.5f ;
	 vec3 norms = normalize(Normals) ;
     vec3 viewDir = normalize(viewPos - FragPos);
	 //ambient shading
	 vec3 ambient = dirLight.ambient * texture(texture1, TexCoords).rgb;     
     vec3 lightDir = normalize(-dirLight.direction);
	 vec3 reflectDir = reflect(-dirLight.direction, norms);

    // diffuse shading
    float diff = max(dot(norms, dirLight.direction), 0.0);
	vec3 diffuse  = dirLight.diffuse  * diff * vec3(texture(texture1, TexCoords));
    // specular shading
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shine);
    vec3 specular = dirLight.specular * spec * vec3(texture(texture1, TexCoords));

    FragColor = vec4(ambient + diffuse + specular,1.0f);

	// was doing shadow caluclation on cube but was not current
	//FragColor = vec4(vec3(shadow),1.0) ;
	//FragColor = texture(texture_diff, vertTex);
}

// function that caluclates shadows
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

