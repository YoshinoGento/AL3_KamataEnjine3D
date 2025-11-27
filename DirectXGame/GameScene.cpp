#include "GameScene.h"
#include "KamataEngine.h"
#include "Math.h"
#include <Windows.h>

using namespace KamataEngine;

void GameScene::Initialize() {

	// モデルのロード
	player_model_ = Model::CreateFromOBJ("player");

	// カメラ初期化
	camera_.Initialize();

	  // カメラ位置をプレイヤーに近づける
	camera_.translation_ = {0.0f, 0.0f, -10.0f}; // Y:高さ、Z:奥行き
	camera_.UpdateMatrix();                     // 行列更新


	// デバッグカメラ作成
	debugCamera_ = new DebugCamera(1280, 720);


	// プレイヤー初期化（座標など）
	player_ = new Player();
	player_->Initialize(player_model_, &camera_, {0.0f, 0.0f, 0.0f});

	//ボス戦の初期化
	enemy_ = new Enemy();
	enemy_->Initialize(Vector3{0.0f, 0.0f, 10.0f});
	
}

void GameScene::Update() {

#ifdef _DEBUG
	//// スペースキーでデバッグカメラ切り替え
	//if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
	//	isDebugCameraActive_ = !isDebugCameraActive_;
	//}
#endif

	if (isDebugCameraActive_) {
		debugCamera_->Update();
		camera_.matView = debugCamera_->GetCamera().matView;
		camera_.matProjection = debugCamera_->GetCamera().matProjection;
		camera_.TransferMatrix();
	} else {
		camera_.UpdateMatrix();
	}

	// プレイヤー更新
	player_->Update();

	// プレイヤーの座標を取得
	const Vector3& playerPosition = player_->GetWorldPosition();

	// ボス更新
	if (enemy_) {
		enemy_->Update(playerPosition);
	}

	// プレイヤー弾 vs ボス部位の当たり判定（今まで通り）
	if (enemy_) {
		const std::list<PlayerBullet*>& bullets = player_->GetBullets();
		for (PlayerBullet* bullet : bullets) {
			if (!bullet || bullet->IsDead()) {
				continue;
			}
			const Vector3& bulletPos = bullet->GetWorldPosition();
			if (enemy_->CheckHit(bulletPos)) {
				bullet->OnHit();
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
}

void GameScene::Draw() {

	// DirectX共通処理取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dモデル描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	// プレイヤー描画
	player_->Draw();

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
