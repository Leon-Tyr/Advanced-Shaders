#include "Terrain.h"


//Terrain constructors
Terrain::Terrain(int widthIn, int depthIn, int stepSizeIn)
{
	width = widthIn;
	depth = depthIn;
	stepSize = stepSizeIn;
	makeVertices(&vertices , 0.f, 0.f);

}



std::vector<float>& Terrain::getVertices() {
	return vertices;
}


void Terrain::makeVertices(std::vector<float> *vertices, float X, float Z) {
	/* triangle a b c
		   b
		   | \
		   a _ c


		 triangle d f e
		   f _ e
			 \ |
			   d

		 c == d
		 b == f
		 Duplicate vertices but easier in long run! (tesselation and LOD)

		a = (x,y,z)
		b = (x, y+1)
		c = (x+1,y)

		d = (x+1,y)
		e = (x, y+1)
		f = (x+1,y+1)

		 each vertex a, b,c, etc. will have 5 data:
		 x y z u v
		  */
	vertices->clear();
	
	for (int y = 0; y < depth -1; y++) {
		
		float  offSetY = Z + ( y * stepSize);
		for (int x = 0; x < width -1 ; x++) {
			float offSetX = X + (x * stepSize);
			makeVertex(offSetX, offSetY, vertices);  // a
			makeVertex(offSetX, offSetY + stepSize, vertices);  // b
			makeVertex(offSetX + stepSize, offSetY, vertices);   // c
			makeVertex(offSetX + stepSize, offSetY, vertices);  //d
			makeVertex(offSetX, offSetY + stepSize, vertices);  //e
			makeVertex(offSetX + stepSize, offSetY + stepSize, vertices);  //f
			
		}
	}
}

void Terrain::makeVertex(int x, int y, std::vector<float> *vertices) {

	//x y z position
	vertices->push_back((float)x); //xPos
	vertices->push_back(0.0f); //yPos - always 0 for now. Going to calculate this on GPU - can change to calclaue it here.
	vertices->push_back((float)y); //zPos

   // add texture coords
	vertices->push_back((float)x / (width*stepSize));
	vertices->push_back((float)y / (depth*stepSize));


}

bool Terrain::checkBounds(float camX, float camZ, float bound)
{
	// get middle x 
	// get middle z
	float middleX = vertices.at(vertices.size() / 2);
	float middleZ = vertices.at((vertices.size() / 2 ) + 2);

	// check if camera is out of set bounds 
	if (camX < (middleX -bound) || camX >(middleX + bound))
	{
		return true;
	}
	if (camZ < (middleZ - bound) || camZ >(middleZ + bound))
	{
		return true;
	}
	return false;
}