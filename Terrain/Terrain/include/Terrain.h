#ifndef TERRAIN_H
#define TERRAIN_H


#include <vector>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "PerlinNoise.h"

class Terrain
{
public:
	Terrain(int widthIn, int heightIn, int stepSizeIn);
	std::vector<float> &getVertices();
	bool checkBounds(float camX, float camZ, float bound);
	void makeVertices(std::vector<float> *vertices, float x, float z);
private:
	std::vector<float> vertices;
	int width;
	int depth;
	int stepSize;
	void makeVertex(int x, int y, std::vector<float> *vertices);
	
};
#endif












