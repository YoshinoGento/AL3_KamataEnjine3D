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

	debugCamera_ = new DebugCamera(1280, 720);
	player_ = new Player();
	player_->SetEnemy(enemy_);

	player_->Initialize(player_model_, &camera_, {0.0f, 0.0f, 0.0f});

	//ボス戦の初期化
	enemy_ = new Enemy();
	enemy_->Initialize(Vector3{0.0f, 0.0f, 10.0f});
	
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


	// ボス更新
	if (enemy_) {
		const std::list<PlayerBullet*>& playerBullets = player_->GetBullets();

		for (PlayerBullet* bullet : playerBullets) {
			if (!bullet) {
				continue;
			}

			// 弾のワールド座標を取得
			const Vector3& bulletPosition = bullet->GetWorldPosition();

			// どれかの部位に当たったら、弾を消す
			if (enemy_->CheckHit(bulletPosition)) {
				bullet->OnHit();
			}
		}
	}
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());

	player_->Draw();
	enemy_->Draw();


	// ボス描画
	if (enemy_) {
		enemy_->Draw(camera_);
	}

	// 3Dモデル描画後処理
	Model::PostDraw();
}

void GameScene::Delete() {
	delete player_;
	delete player_model_;
	delete debugCamera_;
	delete enemy_;
}
