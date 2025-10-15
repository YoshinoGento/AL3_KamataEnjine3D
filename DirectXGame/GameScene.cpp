#include "GameScene.h"

using namespace KamataEngine;

// 初期化
void GameScene::Initialize() {

	camera_ = new Camera();
	camera_->Initialize();
	// ここで終わった10/8日
	//  プレイヤーモデルを読み込み（立方体でも.objでもOK）
	Model* playerModel = Model::CreateFromOBJ("player");

	// プレイヤーの生成
	player_ = new Player();
	player_->Initialize(playerModel, camera_, {0.0f, 0.0f, 10.0f});

	// 敵の生成
	Enemy* enemy = new Enemy();
	enemy->Initialize();
	enemies_.push_back(enemy);
}

// 終了処理
void GameScene::Delete() {

	// プレイヤーの解放
	delete player_;
	player_ = nullptr;

	// 敵の解放
	for (int i = 0; i < enemies_.size(); i++) {
		delete enemies_[i];
	}
	enemies_.clear();

	// 弾の解放
	for (int i = 0; i < bullets_.size(); i++) {
		delete bullets_[i];
	}
	bullets_.clear();
}

// 更新処理
void GameScene::Update() {

	// プレイヤーの更新
	if (player_) {
		player_->Update();
	}

	// 敵更新
	for (int i = 0; i < enemies_.size(); i++) {
		enemies_[i]->Update();
	}

	// 弾更新
	for (int i = 0; i < bullets_.size(); i++) {
		bullets_[i]->Update();
	}
}

// 描画処理
void GameScene::Draw() {

	// プレイヤーの描画
	player_->Drow();

	// 敵の描画
	for (int i = 0; i < enemies_.size(); i++) {
		enemies_[i]->Draw();
	}

	// 弾の描画
	for (int i = 0; i < bullets_.size(); i++) {
		bullets_[i]->Drow();
	}
}
