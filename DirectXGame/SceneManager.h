#pragma once
#include "Scene.h"

class SceneManager {
public:
	void ChangeScene(int sceneID);
	void Update();
	void Draw();
	void Finalize();

private:
	Scene* currentScene_ = nullptr;
};
