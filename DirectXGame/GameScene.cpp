#include "GameScene.h"
#include "KamataEngine.h"
#include "Math.h"
#include <Windows.h>

using namespace KamataEngine;

void GameScene::Initialize() {
	player_model_ = Model::CreateFromOBJ("player1");
	enemy_model_ = Model::CreateFromOBJ("Boss");
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

	
	// ① 先に enemy を作る（Player が参照するので）
	enemy_ = new Enemy();
	player_ = new Player();

	enemy_->Initialize(Vector3{0.0f, 0.0f, 10.0f});
	enemy_->SetPlayer(player_);

	// ② 次に player を作り enemy を渡す
	player_->SetEnemy(enemy_);

	player_->Initialize(player_model_, player_bullet_model_, &PlayerCamera_, {0.0f, 0.0f, 0.0f});

	skydome_model_ = Model::CreateFromOBJ("FaceSkySphere");
	skydome_ = new Skydome;
	skydome_->Initialize(skydome_model_, &camera_);

	 // ★ クリアフラグ初期化
	isGameClear_ = false;
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

	// 天球は毎フレーム更新（マトリクス転送）
	skydome_->Update();

	// プレイヤー更新
	player_->Update();
	const Vector3& playerPos = player_->GetWorldPosition();

	// ============================
	// ボス更新 ＆ 当たり判定（クリア前だけ）
	// ============================
	if (enemy_ && !isGameClear_) {

		// ボス本体の更新（形態遷移などもここ）
		enemy_->Update(playerPos);

		const float kPlayerRadius = 0.5f; // プレイヤー当たり判定の半径

		// ファンネルビーム被弾
		if (enemy_->IsPlayerHitByFunnelBeam(playerPos, kPlayerRadius)) {
			player_->OnHitByBeam();
		}

		// プレイヤー弾 vs ボス部位 ＋ vs 敵ミサイル
		const std::list<PlayerBullet*>& playerBullets = player_->GetBullets();
		const std::list<EnemyBullet*>& enemyBullets = enemy_->GetBullets();

		for (PlayerBullet* bullet : playerBullets) {
			if (!bullet || bullet->IsDead()) {
				continue;
			}

			const Vector3& bulletPos = bullet->GetWorldPosition();

			// ボス部位にヒットしたら弾を消す
			if (enemy_->CheckHit(bulletPos)) {
				bullet->OnHit();
			}

			// 敵ミサイルとの相殺
			for (EnemyBullet* enemyBullet : enemyBullets) {
				if (!enemyBullet || enemyBullet->IsDead()) {
					continue;
				}
				if (enemyBullet->IsHitByPlayerBullet(bulletPos)) {
					bullet->OnHit();
				}
			}
		}

		// 敵ミサイル vs プレイヤー
		for (EnemyBullet* bullet : enemyBullets) {
			if (!bullet || bullet->IsDead()) {
				continue;
			}

			const Vector3& bulletPos = bullet->GetWorldPosition();
			if (player_->IsHitMissile(bulletPos)) {
				// IsHitMissile 内で OnHitByBeam 相当の処理が行われる
			}
		}

		// ============================
		// ゲームクリア判定（ボス撃破）
		// ============================
		if (enemy_->IsDefeated()) {
			isGameClear_ = true;
			// ※ 今はフラグを立てるだけ。
			//    後でここにクリア演出や別シーン遷移を追加できる。
		}
	}

	// ============================
	// 天球の色を形態に応じて変更
	// ============================
	if (enemy_) {
		Enemy::Form form = enemy_->GetForm();

		if (form == Enemy::Form::TWO) {
			// ★ 第二形態：少し赤みを帯びた空
			skydome_->SetTintColor(Vector4{1.0f, 0.6f, 0.6f, 1.0f});
		} else {
			// 第一形態 or 初期：通常色
			skydome_->SetTintColor(Vector4{1.0f, 1.0f, 1.0f, 1.0f});
		}
	}

	// =========================================
	// マウス左クリックで、マウス位置方向に弾を撃つ
	// =========================================
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) { // ボタンが押されている間撃ち続ける

		POINT cursor;
		if (GetCursorPos(&cursor)) {

			const float kWindowWidth = 1280.0f;
			const float kWindowHeight = 720.0f;

			float nx = cursor.x / kWindowWidth;
			float ny = cursor.y / kWindowHeight;

			float sx = nx * 2.0f - 1.0f;
			float sy = 1.0f - ny * 2.0f;

			const float kMoveLimitX = 6.0f; // Player.cpp と同じ値
			const float kMoveLimitY = 3.0f; // Player.cpp と同じ値

			float worldX = sx * kMoveLimitX;
			float worldY = sy * kMoveLimitY;

			Vector3 targetWorld{worldX, worldY, 10.0f};

			player_->FireToward(targetWorld);
		}
	}
}


void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());

	skydome_->Draw();
	enemy_->Draw(camera_);
	player_->Draw();

	// 3Dモデル描画後処理
	Model::PostDraw();
}

void GameScene::Delete() {
	delete player_;
	delete player_model_;
	delete debugCamera_;
	delete enemy_;
	delete skydome_;
}
