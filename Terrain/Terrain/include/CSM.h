#pragma once

//plan to create cascading shadow mapping but was unsuccessful

#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include<iostream>

//#include "Model.h"
#include "Shader.h"


const int n = 3;

/*class CSM
{
public:
	CSM(int width, int height, unsigned int * depthMap, unsigned int * depthMapFBO);
	void setDepthFBO();
	//void createShadowMaps(Shader &depthShader, Model &mod);
	void firstPassFillShadowMaps();
private:
	int SHADOW_WIDTH;
	int SHADOW_HEIGHT;
	unsigned int * depthMap;
	unsigned int * depthMapFBO;

};*/