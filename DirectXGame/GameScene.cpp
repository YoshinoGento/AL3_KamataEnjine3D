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

	skydome_->Update();

	player_->Update();
	const Vector3& playerPos = player_->GetWorldPosition();

	// ボス更新
	if (enemy_) {
		enemy_->Update(playerPos);

		const float kPlayerRadius = 0.5f; // プレイヤー当たり判定の半径（好みで調整）

		if (enemy_->IsPlayerHitByFunnelBeam(playerPos, kPlayerRadius)) {
			// ここで HP 減少とか、被弾リアクションを入れる
			player_->OnHitByBeam();
		}
	}

	// プレイヤー弾 vs ボス部位の当たり判定（今まで通り）
	if (enemy_) {
		const std::list<PlayerBullet*>& bullets = player_->GetBullets();
		const std::list<EnemyBullet*>& enemyBullets = enemy_->GetBullets();
		for (PlayerBullet* bullet : bullets) {
			if (!bullet || bullet->IsDead()) {
				continue;
			}
			const Vector3& bulletPos = bullet->GetWorldPosition();
			if (enemy_->CheckHit(bulletPos)) {
				bullet->OnHit();
			}

			for (EnemyBullet* enemyBullet : enemyBullets) {
				if (!enemyBullet || enemyBullet->IsDead()) {
					continue;
				}

				if (enemyBullet->IsHitByPlayerBullet(bulletPos)) {
					bullet->OnHit();
				}
			}
		}

		for (EnemyBullet* bullet : enemyBullets) {
			if (!bullet || bullet->IsDead()) {
				continue;
			}

			const Vector3& bulletPos = bullet->GetWorldPosition();
			if (player_->IsHitMissile(bulletPos)) {
				player_->OnHitByBeam();
			}
		}
	}

	// =========================================
	// マウス左クリックで、マウス位置方向に弾を撃つ
	// =========================================
	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000) { // ボタンが押されている間撃ち続ける

		POINT cursor;
		if (GetCursorPos(&cursor)) {

			// ここでは簡単のために「ウィンドウ左上が (0,0)、サイズ 1280x720」
			// として扱う（必要なら WinApp からちゃんとクライアント座標を取る形に発展させてOK）
			const float kWindowWidth = 1280.0f;
			const float kWindowHeight = 720.0f;

			// 0〜1 に正規化
			float nx = cursor.x / kWindowWidth;
			float ny = cursor.y / kWindowHeight;

			// -1〜+1 に変換しつつ、Y は上が + になるよう反転
			float sx = nx * 2.0f - 1.0f;
			float sy = 1.0f - ny * 2.0f;

			// プレイヤーの移動制限と対応させてワールド座標にマッピング
			const float kMoveLimitX = 6.0f; // Player.cpp と同じ値
			const float kMoveLimitY = 3.0f; // Player.cpp と同じ値

			float worldX = sx * kMoveLimitX;
			float worldY = sy * kMoveLimitY;

			// 弾の狙い先はボスがいる Z=10 付近の平面上にする
			Vector3 targetWorld{worldX, worldY, 10.0f};

			// プレイヤーからその点に向けて弾を撃つ
			player_->FireToward(targetWorld);
		}
	}

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

	// 形態に応じてスカイドームの色を変える
	if (enemy_ && skydome_) {
		switch (enemy_->GetForm()) {
		case Enemy::Form::ONE:
			// 第一形態：通常の空
			skydome_->SetTintColor(Vector4{1.0f, 1.0f, 1.0f, 1.0f});
			break;
		case Enemy::Form::TWO:
			// 第二形態：少し赤くして不穏感を出す
			skydome_->SetTintColor(Vector4{1.0f, 0.5f, 0.5f, 1.0f});
			break;
		}
	}

	// -------------------------
	// 勝利 / 敗北判定
	// -------------------------

	// ボス撃破 → クリア
	// Enemy::IsDefeated() はコア(0番)破壊で true
	if (enemy_ && enemy_->IsDefeated()) {
		isEnd_ = true;
		nextScene_ = (int)SceneType::CLEAR;
	}

	// プレイヤー HP 0 → ゲームオーバー
	if (player_ && player_->GetHP() <= 0) {
		isEnd_ = true;
		nextScene_ = (int)SceneType::GAMEOVER;
	}

#ifdef _DEBUG
	// --- デバッグ用強制遷移 ---

	// Lキーで強制クリア
	if (GetAsyncKeyState('L') & 0x8000) {
		isEnd_ = true;
		nextScene_ = (int)SceneType::CLEAR;
	}

	// Pキーで強制ゲームオーバー
	if (Input::GetInstance()->TriggerKey(DIK_P)) {
		isEnd_ = true;
		nextScene_ = (int)SceneType::GAMEOVER;
	}
#endif
}

void GameScene::Draw3D() {
	skydome_->Draw();
	enemy_->Draw(camera_);
	player_->Draw();
}

void GameScene::Draw2D() {
	// player_->DrawUI(); // ロックオンUIとか
}

void GameScene::Finalize() {
	delete player_;
	delete enemy_;
	delete skydome_;

	delete player_model_;
	delete enemy_model_;
	delete player_bullet_model_;
	delete skydome_model_;

	delete debugCamera_;
}
