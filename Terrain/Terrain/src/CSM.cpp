#include "CSM.h"

/*CSM::CSM(int width, int height, unsigned int * DMap, unsigned int * DMapFBO)
{
	SHADOW_WIDTH = width;
	SHADOW_HEIGHT = height;
	depthMap = DMap;
	depthMapFBO = DMapFBO;
	setDepthFBO();
}

void CSM::setDepthFBO()
{
	glGenFramebuffers(n, depthMapFBO);

	glGenTextures(n, depthMap);
	for (GLuint i = 0; i < n; i++)
	{
		glBindTexture(GL_TEXTURE_2D, depthMap[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap[i], 0);

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);


	}
}*/

/*void CSM::createShadowMaps(Shader & depthShader, Model & mod)
{
	depthShader.use();
	lightSpaceMatrices.clear();
	for (int i = 0; i < n; i++)
	{
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO[i]);
		glClear(GL_DEPTH_BUFFER_BIT);
		setPlanes(cascadeEnds[i], cascadeEnds[i + 1]);
		lightSpaceMatrices.push_back(getLightSpaceMatrix());
		depthShader.setMat4("lightSpaceMatrix", getLightSpaceMatrix());
		renderScene(depthShader, mod);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
}*/
