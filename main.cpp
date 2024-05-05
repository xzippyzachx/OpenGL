#include "RenderManager.h"
#include <windows.h>

RenderManager* renderManager;

int main()
{
	std::cout << "Start main" << "\n";

	renderManager = new RenderManager();

	double prevTime = 0.0f;
	double currTime = 0.0f;
	double timeDiff;
	unsigned int counter = 0;

	float prevTimeDelta = 0.0f;
	float deltaTime;

	if (renderManager->Init() == -1)
	{
		return -1;
	}

	Model goblin("Resources/Models/Goblin/Goblin.gltf");
	renderManager->AddModel(&goblin);
	Model floor("Resources/Models/Floor/Floor.gltf");
	renderManager->AddModel(&floor);
	Model tree("Resources/Models/Tree/Tree.gltf");
	renderManager->AddModel(&tree);

	tree.position = glm::vec3(0.0f, 0.0f, -4.0f);

	while (!glfwWindowShouldClose(renderManager->GetWindow()))
	{
		currTime = glfwGetTime();
		timeDiff = currTime - prevTime;
		counter++;

		deltaTime = (float)currTime - prevTimeDelta;
		prevTimeDelta = (float)currTime;

		if (timeDiff >= 1.0 / 30.0)
		{
			// Creates new title
			std::string FPS = std::to_string((1.0 / timeDiff) * counter);
			std::string ms = std::to_string((timeDiff / counter) * 1000);
			std::string newTitle = "OpenGL " + FPS + "FPS / " + ms + "ms";
			glfwSetWindowTitle(renderManager->GetWindow(), newTitle.c_str());

			// Resets times and counter
			prevTime = currTime;
			counter = 0;
		}

		renderManager->Render();

		renderManager->GetCamera()->Inputs(renderManager->GetWindow(), deltaTime);
	}

	delete renderManager;

	return 0;
}

