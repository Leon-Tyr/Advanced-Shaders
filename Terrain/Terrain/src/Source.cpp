// current issues
// shadows from terrain not loading in onto terrain
// loading model causes texture of model to attract to terrain similar to cube texture

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Shader.h"
#include "Camera.h"
#include <Model.h>
#include "Terrain.h"
#include "FileSystem.h"
#include "SkyBox.h"
//#include "CSM.h"

#include<string>
#include <iostream>
#include <numeric>

// settings
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;
const unsigned int SHADOW_W = 5120, SHADOW_H = 5120;

// functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(char const * path);
void setVAO(vector <float> vertices);
void ReassignVBO(vector <float> vertices);
void setFBOdepth();
void RenderQuad();
void setRBO();
void setFBOcolour();
void renderCube();
void renderScene(const Shader &shader, std::vector<float> vertices, float time);

// camera
Camera camera(glm::vec3(250, 150, 250));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

//arrays
unsigned int VBO, VAO, FBO, quadVAO = 0, quadVBO;
unsigned int textureDepthBuffer;
unsigned int textureColourbuffer;
unsigned int depthMapFBO;
unsigned int depthMap;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;



int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "IMAT3907", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}


	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glEnable(GL_DEPTH_TEST);

	// loading shaders in below 
	Shader shader("..\\shaders\\plainVert.vs", "..\\shaders\\plainFrag.fs", "..\\shaders\\Norms.gs", "..\\shaders\\tessControlShader.tcs", "..\\shaders\\tessEvaluationShader.tes");
	Shader skyboxShader("..\\shaders\\skybox.vs", "..\\shaders\\skybox.fs", nullptr, nullptr, nullptr);
	Shader PostProcessor("..\\shaders\\postProcess.vs", "..\\shaders\\postProcess.fs", nullptr, nullptr, nullptr);
	Shader ShadowShader("..\\shaders\\depthVert.vs", "..\\shaders\\depthFrag.fs", nullptr, nullptr, nullptr);
	Shader modelShader("..\\shaders\\modelShader.vs", "..\\shaders\\modelShader.fs", nullptr, nullptr, nullptr);

	// Skybox class  object
	SkyBox skybox;

	//Terrain Constructor ; number of grids in width, number of grids in depth, gridSize changed to match X and Z axis. 
	Terrain terrain(50, 50, 10);
	std::vector<float> vertices = terrain.getVertices();
	setVAO(vertices);

	//setFBOcolour();
	setFBOdepth();
	

	//CSM test
	//const int num_cascades = 3;
	//unsigned int depthMapArray[num_cascades];
	//unsigned int depthMapFBOArray[num_cascades];

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << endl;

	//unsigned int heightMap = loadTexture("..\\resources\\heightMap.jpg");
	// load model into world 
	unsigned int TerrainTexture = loadTexture("..\\resources\\metal.jpg");
	unsigned int modelTexture = loadTexture("..\\resources\\iceTexture.png");

	// load textures in for skybox 6 in total, one for each side.
	std::vector<std::string> faces
	{
		FileSystem::getPath("..//..//resources//skybox//right.jpg"),
		FileSystem::getPath("..//..//resources//skybox//left.jpg"),
		FileSystem::getPath("..//..//resources//skybox//top.jpg"),
		FileSystem::getPath("..//..//resources//skybox//bottom.jpg"),
		FileSystem::getPath("..//..//resources//skybox//front.jpg"),
		FileSystem::getPath("..//..//resources//skybox//back.jpg")
	};
	unsigned int cubemapTexture = skybox.loadCubemap(faces);//textures are then sent to the function to load on to the skybox

	// obtain single frame of time for perlin noise
	float OneTimeFrame = glfwGetTime();

	// light pos
	glm::vec3 dirLightPos(2.0, 2.5f, 2.0f);

	// set up texture for main shader
	shader.use();
	shader.setInt("texture1", 0);
	shader.setInt("shadowMap", 1); // changing value to 0 show different shadows by cube shadow disappear 

	// set up texture for skybox shader
	skyboxShader.use(); //use skybox shader
	skyboxShader.setInt("skybox", 0);

	// set up texture and near/far plane values for post-processing shader
	PostProcessor.use();
	PostProcessor.setInt("scene", 0);
	PostProcessor.setFloat("near_plane", 0.1f);
	PostProcessor.setFloat("far_plane", 1000.0f);

	// set up texture for model shader
	modelShader.use();
	modelShader.setInt("texture1", 0);
	modelShader.setInt("shadowMap", 0);

	bool generateTerrain = false;
	int shadowOn;
	bool ppColour = false;
	bool ppInverse = false;
	

	// load model nanasuit 
	// not used because caused issue where texture would load onto terrain
	//Model m_model("..\\resources\\nano\\nanosuit\\nanosuit.obj");

	while (!glfwWindowShouldClose(window))
	{
		// keypress to switch shadows on and off
		shadowOn = 1;
		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
			shadowOn = 0;
		else 
			shadowOn = 1;

		// keypress to change to wireframe
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		//keypress with Post-Processing to switch between pp colour and inverse colour
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			ppInverse = true;
		else
			ppInverse = false;

		// timer
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);

		// infinite terrain, calculate bounds and vertices
		if (terrain.checkBounds(camera.Position.x, camera.Position.z, 30.f) && generateTerrain == false)//check Bounds
		{
			terrain.makeVertices(&terrain.getVertices(), camera.Position.x - 250.0f, camera.Position.z - 250.0f); //create new vertices from camera position
			vertices = terrain.getVertices();// reassign vertices
			ReassignVBO(vertices);	// rebind VBO - generate new VBO - unbind vbo
			generateTerrain = true;

		} // bool to stop generating terrain
		if (generateTerrain == true)
		{
			generateTerrain = false;
		}

		// used to switch between pp and normal
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glEnable(GL_DEPTH_TEST);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// MVP properties 
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		glm::vec3 eyePos = camera.Position;
		glm::mat4 model = glm::mat4(1.0f);

		//main shader uniform data
		shader.use();
		shader.setMat4("projection", projection);
		shader.setMat4("view", view);
		shader.setVec3("eyePos", eyePos);
		shader.setInt("heightScale", 80);
		shader.setFloat("time", OneTimeFrame);
		shader.setInt("shadowOn", shadowOn);

		//light properties for blinn phong
		shader.setVec3("viewPos", camera.Position);
		shader.setVec3("dirLight.direction", dirLightPos);
		shader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
		shader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
		shader.setVec3("dirLight.specular", 0.7f, 0.7f, 0.7f);

		// light data
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 0.1f, far_plane = 1000.0f;

		//light space matrix for shadow mapping
		lightProjection = glm::ortho(-250.0f,  250.0f,-250.0f, 250.0f, near_plane, far_plane);
		glm::vec3 lightPos, lookingAt;
		lightPos = glm::vec3(80, 300, 150);
		lookingAt = glm::vec3(-50, 150,150);
		lightView = glm::lookAt(lightPos, lookingAt, glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;

		//model shader uniform data
		modelShader.use();
		modelShader.setMat4("projection", projection);
		modelShader.setMat4("view", view);
		modelShader.setVec3("viewPos", camera.Position);
		modelShader.setVec3("dirLight.direction", dirLightPos);
		modelShader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
		modelShader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
		modelShader.setVec3("dirLight.specular", 0.7f, 0.7f, 0.7f);
		modelShader.setInt("shadowOn", shadowOn);

		PostProcessor.use();
		PostProcessor.setBool("PPinverse", ppInverse);

		//draw to scene first 
		//terrain
		shader.use();
		shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TerrainTexture);
		renderScene(shader, vertices, currentFrame); 
		//model
		modelShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, modelTexture);
		renderScene(modelShader, vertices, currentFrame);

		//first pass for shadow
		glViewport(0, 0, SHADOW_W, SHADOW_H);
		ShadowShader.use();
		ShadowShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderScene(ShadowShader, vertices, currentFrame);

		//reset viewport
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//second pass shadow using model shader
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		modelShader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, modelTexture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
		renderScene(modelShader, vertices, currentFrame);

		//draw terrain again so it can switch between pp and normal
		shader.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TerrainTexture);
		renderScene(shader, vertices, currentFrame);

		//skybox pass - always last
		glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
		skyboxShader.use();
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		skybox.draw(cubemapTexture);
		glDepthFunc(GL_LESS); //uncomment for shadow 

		// keypress to switch between post-processing and normal 
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
		{
			ppColour = true;
			//PostProcessing 
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDisable(GL_DEPTH_TEST);
			glClear(GL_COLOR_BUFFER_BIT);
			PostProcessor.use();
			glBindVertexArray(quadVAO);
			glBindTexture(GL_TEXTURE_2D, textureColourbuffer);
			RenderQuad();
			glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		}
		else 
		{ 
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glBindVertexArray(0);
		}
		if (ppColour == true) 
		{ 
			// enable colour buffers
			setFBOcolour(); 
			ppColour = false;
		}

		// keypress to check camera position
		if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
			camera.printCameraCoords();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	skybox.DeleteBuffer();
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

// load texture
unsigned int loadTexture(char const * path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		stbi_image_free(data);
		std::cout << "Loaded texture at path: " << path << " width " << width << " id " << textureID << std::endl;

	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);

	}

	return textureID;
}

// set VAO to create terrain 
void setVAO(vector <float> vertices) {

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(GLfloat)), vertices.data(), GL_DYNAMIC_DRAW);

	//xyz
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//texture
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// reassign VBO when new terrain is set
void ReassignVBO(vector <float> vertices)
{
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(GLfloat)), vertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// set colour framebuffer
void setFBOcolour()
{
	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);

	glGenTextures(1, &textureColourbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColourbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, textureColourbuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColourbuffer, 0);
	setRBO();
}

// set depth framebuffer - currently used for shadows
void setFBOdepth()
{
	glGenFramebuffers(1, &depthMapFBO);

	glGenTextures(1, &textureDepthBuffer);
	glBindTexture(GL_TEXTURE_2D, textureDepthBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepthBuffer, 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

// render a quad for post-processing
void RenderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] =
		{
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,

		};
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// set render buffer 
void setRBO()
{
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERER, rbo);
}

// function to load cubes in
unsigned int cubeVAO;
unsigned int cubeVBO;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// render scene for cubes and terrain
void renderScene(const Shader &shader, std::vector<float> vertices, float time)
{
	glm::mat4 model = glm::mat4(1.0f);
	shader.setMat4("model", model);

	//render terrain
	glBindVertexArray(VAO);
	glDrawArrays(GL_PATCHES, 0, vertices.size() / 3);
	glBindVertexArray(0);

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(200.0f, 120.0f, 150.0f));
	model = glm::rotate(model, glm::radians(60.f), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
	model = glm::scale(model, glm::vec3(20.0f));
	shader.setMat4("model", model);
	renderCube();

	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(200.0f, 150.0f, 250.0f));
	model = glm::rotate(model, glm::radians(time * 8), glm::normalize(glm::vec3(0.0, 1.0, 0.0)));
	model = glm::scale(model, glm::vec3(10.0f));
	shader.setMat4("model", model);
	renderCube();
	// cubes

}

