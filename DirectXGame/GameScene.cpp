#include "GameScene.h"
#include "KamataEngine.h"
#include "Math.h"



using namespace KamataEngine;

void GameScene::Initialize() {
	player_model_ = Model::CreateFromOBJ("player");
	enemy_model_ = Model::CreateFromOBJ("enemy");

	uint32_t lockonTexture = TextureManager::Load("lockon_br.png");

	// ★ camera_ だけ使う
	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();

	// 敵を共通 camera_ で初期化
	for (int i = 0; i < 5; ++i) {
		Enemy* enemy = new TacklerEnemy();
		Vector3 position = {float(i * 3 - 6), 0.0f, 10.0f};
		enemy->Initialize(enemy_model_, &camera_, position);
		enemies_.push_back(enemy);
	}



	// デバッグカメラは今のままでOK
	debugCamera_ = new DebugCamera(1280, 720);

	// プレイヤーも共通 camera_ で初期化
	player_ = new Player();
	player_->Initialize(player_model_, &camera_, {0, 0, 0}, lockonTexture);

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

		if (enemy->CheckCollision(player_->GetWorldPosition(), player_->GetRadius())) {

			player_->OnHitByBeam(); // or OnHit()
		}
	}
}

void GameScene::Draw() {

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ---------- 3D描画 ----------
	Model::PreDraw(dxCommon->GetCommandList());

	player_->Draw3D(); // ← 3D専用の描画関数
	for (Enemy* enemy : enemies_) {
		enemy->Draw3D();
	}

	Model::PostDraw();

	// ---------- 2D描画 ----------
	Sprite::PreDraw(dxCommon->GetCommandList());

	player_->Draw2D(); // ← ロックオンマーカーなど
	// HPバー、UI 等もここ

	Sprite::PostDraw();
}

void GameScene::Delete() {

	// 敵を全削除
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	delete player_;
	player_ = nullptr;

	delete debugCamera_;
	debugCamera_ = nullptr;

	delete player_model_;
	delete enemy_model_;

	player_model_ = nullptr;
	enemy_model_ = nullptr;
}
