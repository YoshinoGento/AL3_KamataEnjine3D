#include "GameScene.h"
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

void GameScene::Initialize() {
	player_model_ = Model::CreateFromOBJ("player1");
	enemy_model_ = Model::CreateFromOBJ("enemy");
	player_bullet_model_ = Model::CreateFromOBJ("PlayerBullet");


	PlayerCamera_.Initialize();
	EnemyCamera_.Initialize();
	camera_.Initialize();

	PlayerCamera_.translation_ = {0.0f, 0.0f, -10.0f};
	PlayerCamera_.UpdateMatrix();
	EnemyCamera_.translation_ = {0.0f, -1.0f, -30.0f};
	EnemyCamera_.UpdateMatrix();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();

	debugCamera_ = new DebugCamera(1280, 720);
	player_ = new Player();
	player_->Initialize(player_model_,player_bullet_model_, &PlayerCamera_, {0.0f, 0.0f, 0.0f});

	enemy_ = new Enemy();
	enemy_->Initialize(enemy_model_, &EnemyCamera_, {0.0f, 0.0f, 10.0f});
	player_->SetEnemy(enemy_);
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
	enemy_->Update(player_->GetWorldPosition());
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());

	player_->Draw();
	enemy_->Draw();

	Model::PostDraw();
}

void GameScene::Delete() {
	delete player_;
	delete player_model_;
	delete debugCamera_;
}
