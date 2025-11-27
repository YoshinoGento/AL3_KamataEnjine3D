#include "GameScene.h"
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

void GameScene::Initialize() {
	player_model_ = Model::CreateFromOBJ("player");
	enemy_model_ = Model::CreateFromOBJ("enemy");

	PlayerCamera_.Initialize();
	EnemyCamera_.Initialize();
	camera_.Initialize();

	PlayerCamera_.translation_ = {0.0f, 0.0f, -10.0f};
	PlayerCamera_.UpdateMatrix();
	EnemyCamera_.translation_ = {0.0f, -1.0f, -30.0f};
	EnemyCamera_.UpdateMatrix();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();


	//モデルなどは先に読込先と仮定
	for (int i = 0; i < 5; ++i) {
		Enemy* enemy_ = new Enemy();
		Vector3 position = {float(i * 3 - 6), 0.0f, 10.0f}; //横一列
		enemy_->Initialize(enemy_model_, &EnemyCamera_, position);
		enemies_.push_back(enemy_);
	}



	debugCamera_ = new DebugCamera(1280, 720);
	player_ = new Player();
	player_->Initialize(player_model_, &PlayerCamera_, {0.0f, 0.0f, 0.0f});

	player_->SetEnemies(enemies_);
}

void GameScene::Update() {
#ifdef _DEBUG
	// if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
	//     isDebugCameraActive_ = !isDebugCameraActive_;
	// }
#endif
	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}

	player_->Update();
	for (Enemy* enemy : enemies_) {
		enemy->Update(player_->GetWorldPosition());
	}
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());

	player_->Draw();
	for (Enemy* enemy_ : enemies_) {
		enemy_->Draw();
	}


	Model::PostDraw();
}

void GameScene::Delete() {
	delete player_;
	delete player_model_;
	delete debugCamera_;
}
