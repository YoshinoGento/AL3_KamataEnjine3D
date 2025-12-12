#include "GameScene.h"
#include "KamataEngine.h"
#include "Math.h"

using namespace KamataEngine;

void GameScene::Initialize() {
	player_model_ = Model::CreateFromOBJ("player");
	enemy_model_ = Model::CreateFromOBJ("enemy");

	// ★ camera_ だけ使う
	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();

	// 敵を共通 camera_ で初期化
	for (int i = 0; i < 5; ++i) {
		Enemy* enemy = new Enemy();
		Vector3 position = {float(i * 3 - 6), 0.0f, 10.0f};
		enemy->Initialize(enemy_model_, &camera_, position);
		enemies_.push_back(enemy);
	}

	// デバッグカメラは今のままでOK
	debugCamera_ = new DebugCamera(1280, 720);

	// プレイヤーも共通 camera_ で初期化
	player_ = new Player();
	player_->Initialize(player_model_, &camera_, {0.0f, 0.0f, 0.0f});

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
