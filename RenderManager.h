#ifndef RENDERMANAGER_CLASS_H
#define RENDERMANAGER_CLASS_H

#include"Model.h"

class RenderManager
{
public:
	RenderManager();

	int Init();

	void Render();

	void AddModel(Model* model);
	void RemoveModel(Model* model);

	GLFWwindow* GetWindow();
	Camera* GetCamera();

	~RenderManager();
protected:
	std::vector<Model*> models;
};

#endif