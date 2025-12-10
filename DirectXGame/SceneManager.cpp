#include "SceneManager.h"
#include "ClearScene.h"
#include "GameOverScene.h"
#include "GameScene.h"
#include "TitleScene.h"


void SceneManager::Initialize() {

	ChangeScene((int)SceneType::TITLE); 

}

void SceneManager::ChangeScene(int sceneID) {
	if (currentScene_) {
		currentScene_->Finalize();
		delete currentScene_;
	}

	switch ((SceneType)sceneID) {
	case SceneType::TITLE:
		currentScene_ = new TitleScene();
		break;
	case SceneType::GAME:
		currentScene_ = new GameScene();
		break;
	case SceneType::CLEAR:
		currentScene_ = new ClearScene();
		break;

	case SceneType::GAMEOVER:
		currentScene_ = new GameOverScene();
		break;
	}
	currentScene_->Initialize();
}

void SceneManager::Update() {
	currentScene_->Update();

	if (currentScene_->IsEnd()) {
		ChangeScene(currentScene_->NextScene());
	}
}

void SceneManager::Draw() {
	auto* dx = DirectXCommon::GetInstance();
	auto* cmd = dx->GetCommandList();

	// --- 3D ---
	Model::PreDraw(cmd);
	currentScene_->Draw3D();
	Model::PostDraw();

	// --- 2D ---
	Sprite::PreDraw(cmd);
	currentScene_->Draw2D();
	Sprite::PostDraw();
}

void SceneManager::Finalize() {
	if (currentScene_) {
		currentScene_->Finalize();
		delete currentScene_;
		currentScene_ = nullptr;
	}
}