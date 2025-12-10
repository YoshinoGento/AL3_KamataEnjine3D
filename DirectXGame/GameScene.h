#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include <memory> // スマートポインタを使うなら
#include "Skydome.h"
#include "SceneType.h"
#include "Scene.h"

using namespace KamataEngine;

class GameScene : public Scene {
public:
	void Initialize() override;
	void Update() override;
	void Draw3D() override;
	void Draw2D() override; // ← UI ここに移動
	void Finalize() override;

	bool IsEnd() override { return isEnd_; }
	int NextScene() override { return nextScene_; }

private:
	Player* player_ = nullptr;
	Enemy* enemy_ = nullptr;
	Model* player_model_ = nullptr;
	Model* enemy_model_ = nullptr;
	Model* player_bullet_model_ = nullptr;

	Camera camera_;
	Camera PlayerCamera_;
	Camera EnemyCamera_;

	uint32_t textureHandle_ = 0u;
	DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	Skydome* skydome_ = nullptr;
	Model* skydome_model_ = nullptr;

	bool isEnd_ = false;
	int nextScene_ = (int)SceneType::CLEAR;
};
