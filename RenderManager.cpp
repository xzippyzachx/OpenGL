#include "RenderManager.h"

int windowWidth = 1280;
int windowHeight = 720;
Camera* camera;

unsigned int samples = 8;
float gamma = 2.2f;

GLFWwindow* window;

// Generates Shaders
Shader* shaderProgram;
Shader* outliningProgram;
Shader* shadowMapProgram;

// Take care of all the light related things
glm::vec4 lightColor;
glm::vec3 lightPos;

// Matrices needed for the light's perspective
glm::mat4 orthgonalProjection;
glm::mat4 lightView;
glm::mat4 lightProjection;

unsigned int shadowMapWidth = 4096, shadowMapHeight = 4096;
unsigned int shadowMap;
unsigned int shadowMapFBO;

RenderManager::RenderManager()
{
	camera = new Camera(windowWidth, windowHeight, glm::vec3(0.0f, 2.0f, 2.0f));

	// Take care of all the light related things
	lightColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightPos = glm::vec3(1.0f, 1.0f, 1.0f);

	// Matrices needed for the light's perspective
	orthgonalProjection = glm::ortho(-5.0f, 5.0f, -5.0f, 5.0f, 0.1f, 75.0f);
	lightView = glm::lookAt(25.0f * lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	lightProjection = orthgonalProjection * lightView;

	glfwInit();

	// Tell GLFW what version of OpenGL we are using 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, samples);
	// Tell GLFW we are using the CORE profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowWidth, windowHeight, "OpenGL", NULL, NULL);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0, 0, width, height);
	camera->UpdateWindow(width, height);
}

int RenderManager::Init()
{
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	//Load GLAD so it configures OpenGL
	gladLoadGL();
	// Specify the viewport of OpenGL in the Window
	glViewport(0, 0, windowWidth, windowHeight);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	shaderProgram = new Shader("Resources/Shaders/default.vert", "Resources/Shaders/default.frag");
	shadowMapProgram = new Shader("Resources/Shaders/shadowMap.vert", "Resources/Shaders/shadowMap.frag");

	shaderProgram->Activate();
	glUniform4f(glGetUniformLocation(shaderProgram->ID, "lightColor"), lightColor.x, lightColor.y, lightColor.z, lightColor.w);
	glUniform3f(glGetUniformLocation(shaderProgram->ID, "lightPos"), lightPos.x, lightPos.y, lightPos.z);

	// Enables the Depth Buffer
	glEnable(GL_DEPTH_TEST);

	glEnable(GL_MULTISAMPLE);
	glEnable(GL_FRAMEBUFFER_SRGB);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glFrontFace(GL_CCW);

	// Set Vsync
	glfwSwapInterval(1);

	// Framebuffer for Shadow Map
	glGenFramebuffers(1, &shadowMapFBO);

	// Texture for Shadow Map FBO
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	// Prevents darkness outside the frustrum
	float clampColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, clampColor);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	// Needed since we don't touch the color buffer
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	shadowMapProgram->Activate();
	glUniformMatrix4fv(glGetUniformLocation(shadowMapProgram->ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));

	return 0;
}

void RenderManager::Render()
{
	// Depth testing needed for Shadow Map
	glEnable(GL_DEPTH_TEST);

	// Preparations for the Shadow Map
	glViewport(0, 0, shadowMapWidth, shadowMapHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	// Draw scene for shadow map
	for (Model* model : models)
	{
		model->Draw(*shadowMapProgram, *camera);
	}

	// Switch back to the default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// Switch back to the default viewport
	glViewport(0, 0, windowWidth, windowHeight);

	// Specify the color of the background
	glClearColor(pow(0.4f, gamma), pow(0.74f, gamma), pow(1.0f, gamma), 1.0f);
	// Clean the back buffer and assign the new color to it
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	camera->updateMatrix(80.0f, 0.1f, 100.0f);

	shaderProgram->Activate();
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram->ID, "lightProjection"), 1, GL_FALSE, glm::value_ptr(lightProjection));

	// Bind the Shadow Map
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glUniform1i(glGetUniformLocation(shaderProgram->ID, "shadowMap"), 2);

	// Draw models
	for (Model* model : models)
	{
		model->Draw(*shaderProgram, *camera);
	}

	// Swap the back buffer with the front buffer
	glfwSwapBuffers(window);
	// Take care of all GLFW events
	glfwPollEvents();
}

void RenderManager::AddModel(Model* model)
{
	models.push_back(model);
}

void RenderManager::RemoveModel(Model* model)
{
	auto index = find(models.begin(), models.end(), model);
	if (index != models.end())
	{
		models.erase(index);
	}
}

GLFWwindow* RenderManager::GetWindow()
{
	return window;
}

Camera* RenderManager::GetCamera()
{
	return camera;
}

RenderManager::~RenderManager()
{
	// Delete all the objects we've created
	shaderProgram->Delete();
	delete shaderProgram;
	shadowMapProgram->Delete();
	delete shadowMapProgram;

	// Delete window before ending the program
	glfwDestroyWindow(window);
	// Terminate GLFW before ending the program
	glfwTerminate();

	models.clear();
	delete camera;
}