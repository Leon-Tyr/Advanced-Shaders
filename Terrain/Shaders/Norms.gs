#version 330 core
layout(triangles) in ;
layout(triangle_strip, max_vertices = 3) out ;
vec3 getNormal() ;

in vec3 posES[] ;
in vec2 TESTexCoords[];
in vec3 normES[];
in float visibility[];
//in vec4 FPLightSpace[];

out vec3 gNormals ;
out vec3 gWorldPos_FS_in ;
out vec2 gTexCoords;
out float Gvisibility;
//out vec4 gFragPosLightSpace;

void main()
{
   for(int i = 0 ; i < 3; i++)
   {
      gl_Position = gl_in[i].gl_Position ;
      gWorldPos_FS_in = posES[i] ;
	  gTexCoords = TESTexCoords[i];
      gNormals = getNormal();   
	  Gvisibility = visibility[i];
	 // gFragPosLightSpace = FPLightSpace[i];
      EmitVertex() ;
  }
     EndPrimitive() ;

}


vec3 getNormal()
{
    vec3 a = vec3(gl_in[1].gl_Position) - vec3(gl_in[0].gl_Position);
    vec3 b = vec3(gl_in[2].gl_Position) - vec3(gl_in[0].gl_Position);
    return normalize(cross(a, b));
}
