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

	// ============================
	// ★ BGM 読み込み＆再生
	// ============================
	auto* audio = Audio::GetInstance();

	bgmDataPhase1_ = audio->LoadWave("firstForm.wav");
	bgmDataPhase2_ = audio->LoadWave("Second form.wav");

	// 最初は形態 ONE
	bgmVoiceHandle_ = audio->PlayWave(bgmDataPhase1_, true);
	lastForm_ = Enemy::Form::ONE;

	playerHealth_model_ = Model::Create();
	worldTransformPlayerHealth_.Initialize();
	worldTransformPlayerHealth_.translation_ = {0.0f, -3.0f, 0.0f};
	worldTransformPlayerHealth_.scale_ = {0.1f, 0.1f, 0.1f};

	for (int i = 0; i < 3; i++) {
		enemyHealth_model_[i] = Model::Create();
		worldTransformEnemyHealth_[i].Initialize();
		worldTransformEnemyHealth_[i].scale_ = {0.1f, 0.1f, 0.1f};
	}

	worldTransformEnemyHealth_[0].translation_ = {0.0f, 3.0f, 0.0f};
	worldTransformEnemyHealth_[1].translation_ = {-2.0f, 2.0f, 0.0f};
	worldTransformEnemyHealth_[2].translation_ = {2.0f, 2.0f, 0.0f};

	health_texture = TextureManager::Load("White1x1.png");
	playerHealthBarColor.Initialize();
	playerHealthBarColor.SetColor({1.0f, 1.0f, 0.0f, 1.0f});

	enemyHealthBarColor.Initialize();
	enemyHealthBarColor.SetColor({255.0f, 0.0f, 0.0f, 1.0f});

	//------ポーズメニュー初期化------------
	uint32_t menuTex = TextureManager::Load("menu_bg.png");
	menuBG_ = Sprite::Create(menuTex, {640, 360});
	menuBG_->SetAnchorPoint({0.5f, 0.5f});

	// ハイライト（細い青帯）
	uint32_t hlTex = TextureManager::Load("highlight.png");
	highlight_ = Sprite::Create(hlTex, {640, 330});
	highlight_->SetAnchorPoint({0.5f, 0.5f});

	// つづける
	uint32_t contTex = TextureManager::Load("continue.png");
	continueText_ = Sprite::Create(contTex, {640, 330});
	continueText_->SetAnchorPoint({0.5f, 0.5f});

	// タイトルへ
	uint32_t titleTex = TextureManager::Load("totitle.png");
	titleText_ = Sprite::Create(titleTex, {640, 420});
	titleText_->SetAnchorPoint({0.5f, 0.5f});

	// カーソル（三角）
	uint32_t curTex = TextureManager::Load("cursor.png");
	cursor_ = Sprite::Create(curTex, {500, 330});
	cursor_->SetAnchorPoint({0.5f, 0.5f});
}

void GameScene::Update() {

	// ====== ポーズ切替 ======
	if (Input::GetInstance()->TriggerKey(DIK_M)) {
		if (state_ == GameState::Play) {
			state_ = GameState::Pause;
			return; // 以降のゲーム更新をしない
		} else {
			state_ = GameState::Play;
			return;
		}
	}

	if (state_ == GameState::Pause) {

		// 上下で選択切替
		if (Input::GetInstance()->TriggerKey(DIK_UP)) {
			menuIndex_--;
			if (menuIndex_ < 0)
				menuIndex_ = 1;
		}

		if (Input::GetInstance()->TriggerKey(DIK_DOWN)) {
			menuIndex_++;
			if (menuIndex_ > 1)
				menuIndex_ = 0;
		}

		// -------- カーソル位置更新 --------
		if (menuIndex_ == 0) {
			highlight_->SetPosition({640, 330});
			cursor_->SetPosition({430, 330}); // ← 位置を左へずらした
		}
		if (menuIndex_ == 1) {
			highlight_->SetPosition({640, 420});
			cursor_->SetPosition({430, 420}); // ← 位置を左へずらした
		}

		// -------- 決定（Enter） --------
		if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
			if (menuIndex_ == 0) {
				// 続ける
				state_ = GameState::Play;
			} else if (menuIndex_ == 1) {
				// タイトルへ
				isEnd_ = true;
				nextScene_ = (int)SceneType::TITLE;
			}
		}

		return; // ★ ゲーム更新は一切しない！
	}



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

	worldTransformPlayerHealth_.scale_.x = float(player_->GetHP()) / 2.0f;
	WorldTransformUpdate(worldTransformPlayerHealth_);

	for (int i = 0; i < 3; i++) {
		worldTransformEnemyHealth_[i].scale_.x = float(enemy_->GetHP(i)) / 2.0f;
		WorldTransformUpdate(worldTransformEnemyHealth_[i]);
	}

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


	// ===============================
	// ★ 形態変化による BGM 切り替え
	// ===============================
	if (enemy_) {

		auto* audio = Audio::GetInstance();
		Enemy::Form currentForm = enemy_->GetForm(); // ← Getter が必要

		if (currentForm != lastForm_) {

			// いま鳴ってるBGMを止める
			audio->StopWave(bgmVoiceHandle_);

			// 新しい形態のBGMを再生して、その再生IDを保持
			if (currentForm == Enemy::Form::ONE) {
				bgmVoiceHandle_ = audio->PlayWave(bgmDataPhase1_, true);
			} else {
				bgmVoiceHandle_ = audio->PlayWave(bgmDataPhase2_, true);
			}

			lastForm_ = currentForm;
		}
	}

	//// 例えば敵を倒したらクリア
	// if (enemy_->IsDead()) {
	//	isEnd_ = true;
	// }
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

	playerHealth_model_->Draw(worldTransformPlayerHealth_, camera_, health_texture, &playerHealthBarColor);

	for (int i = 0; i < 3; i++) {
		enemyHealth_model_[i]->Draw(worldTransformEnemyHealth_[i], camera_, health_texture, &enemyHealthBarColor);
	}
}

void GameScene::Draw2D() {

	// --- 通常UI（ロックオンUIなど） ---
	// player_->DrawUI();

	// --- ポーズ中だけメニュー表示 ---
	if (state_ == GameState::Pause) {

		menuBG_->Draw();       // 背景（青系の半透明など）
		highlight_->Draw();    // 選択ハイライト帯
		continueText_->Draw(); // 「つづける」
		titleText_->Draw();    // 「タイトルへ」
		cursor_->Draw();       // 三角カーソル
	}
}

void GameScene::Finalize() {

	auto* audio = Audio::GetInstance();
	audio->StopWave(bgmVoiceHandle_);

	// --- ポーズメニューの破棄 ---
	delete menuBG_;
	delete highlight_;
	delete continueText_;
	delete titleText_;
	delete cursor_;

	// --- ゲーム関連 ---
	delete player_;
	delete enemy_;
	delete skydome_;

	delete player_model_;
	delete enemy_model_;
	delete player_bullet_model_;
	delete skydome_model_;

	delete debugCamera_;

	delete playerHealth_model_;
	delete enemyHealth_model_;
}
