#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include <memory> // スマートポインタを使うなら
#include "Skydome.h"

using namespace KamataEngine;

class GameScene {
public:
	void Initialize();
	void Update();
	void Draw();
	void Delete();

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


};
