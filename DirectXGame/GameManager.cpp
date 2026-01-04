#include "GameManager.h"
#include "ClearScene.h"
#include "GameOverScene.h"
#include "GameScene.h"
#include "IScene.h"
#include "TitleScene.h"

void GameManager::Initialize() { ChangeScene(SceneType::Title); }

void GameManager::Update() {
	if (scene_)
		scene_->Update();

	if (nextSceneRequested_) {
		nextSceneRequested_ = false;
		ChangeScene(nextScene_);
	}
}

void GameManager::Draw() {
	if (scene_)
		scene_->Draw();
}

void GameManager::Finalize() {
	if (scene_) {
		scene_->Finalize();
		scene_.reset();
	}
}

void GameManager::ChangeScene(SceneType type) {
	if (scene_) {
		scene_->Finalize();
		scene_.reset();
	}

	current_ = type;

	switch (type) {
	case SceneType::Title:
		scene_ = std::make_unique<TitleScene>();
		break;
	case SceneType::Game:
		scene_ = std::make_unique<GameScene>();
		break;
	case SceneType::Clear:
		scene_ = std::make_unique<ClearScene>();
		break;
	case SceneType::GameOver:
		scene_ = std::make_unique<GameOverScene>();
		break;
	}

	scene_->Initialize(this);
}
