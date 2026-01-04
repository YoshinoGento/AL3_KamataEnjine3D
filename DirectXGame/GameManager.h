#pragma once
#include <memory>
#include "IScene.h"

enum class SceneType {
	Title,
	Game,
	Clear,
	GameOver
};

class GameManager {
public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	// シーンから「次はこれに行きたい」を要求する
	void RequestChangeScene(SceneType next) {
		nextSceneRequested_ = true;
		nextScene_ = next;
	}

private:
	void ChangeScene(SceneType type);

private:
	std::unique_ptr<IScene> scene_;
	SceneType current_ = SceneType::Title;

	bool nextSceneRequested_ = false;
	SceneType nextScene_ = SceneType::Title;
};
