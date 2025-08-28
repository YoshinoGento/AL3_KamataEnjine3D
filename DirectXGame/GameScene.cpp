#include "GameScene.h"
#include "Math.h"

using namespace KamataEngine;

// エフェクトを生成
void GameScene::CreateEffect(const Vector3& position) {

	HitEffect* newHitEffect = HitEffect::Create(position);

	hitEffects_.push_back(newHitEffect);
}

GameScene::~GameScene() {

	delete sprite_;
	delete model_;

	delete block_model_;
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete debugCamera_;
	delete modelSkydome_;
	delete mapChipField_;

	// 02_09 10枚目 敵クラス削除→02_10 6枚目で削除
	//	delete enemies_;

	// 02_10 6枚目 敵クラス削除
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}

	// 02_11_17枚目
	delete deathParticles_;
	delete deathParticle_model_;

	// 02_16 17枚目
	for (HitEffect* hitEffect : hitEffects_) {
		delete hitEffect;
	}
}

void GameScene::Initialize() {

	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("sample.png");
	// スプライト生成
	sprite_ = Sprite::Create(textureHandle_, {100, 50});
	// 3Dモデル生成
	model_ = Model::Create();
	// ワールドトランスフォーム初期化
	worldTransform_.Initialize();

	// カメラ初期化
	camera_.Initialize();

	// ブロックモデル
	block_model_ = Model::CreateFromOBJ("block");

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(WinApp::kWindowWidth, WinApp::kWindowHeight);

	// 02_03 skydome生成
	skydome_ = new Skydome();
	// 初期化
	modelSkydome_ = Model::CreateFromOBJ("SkyDome", true);
	skydome_->Initialize(modelSkydome_, &camera_);

	// 02_04マップチップ
	mapChipField_ = new MapChipField;
	mapChipField_->LoadMapChipCsv("Resources/blocks.csv");
	GenerateBlocks();

	// 02_07 マップチップクラスを作ってからプレイヤークラスを作る
	// という順番に入れ替える
	// 02_01から追加 プレイヤー生成
	player_ = new Player();

	// プレイヤーモデル
	player_model_ = Model::CreateFromOBJ("player");
	Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(2, 18);
	// 02_14 34枚目 プレイヤー攻撃エフェクトモデル
	modelAttack_ = Model::CreateFromOBJ("attack_effect");

	// 02_07 スライド5枚目
	player_->SetMapChipField(mapChipField_);

	// 02_14 34枚目でプレイヤー攻撃エフェクト引数追加
	player_->Initialize(player_model_, modelAttack_, &camera_, playerPosition);

	// 02_06カメラコントローラ スライド13枚目
	CController_ = new CameraController(); // 生成
	CController_->Initialize(&camera_);    // 初期化
	CController_->SetTarget(player_);      // 追従対象セット
	CController_->Reset();                 // リセット

	// 02_06カメラコントローラ スライド18枚目
	CameraController::Rect cameraArea = {12.0f, 100 - 12.0f, 6.0f, 6.0f};
	CController_->SetMovableArea(cameraArea);

	// 02_09 10枚目 敵クラス → 02_10の5枚目で削除
	//	enemy_ = new Enemy();
	// 02_09 10枚目 敵モデル
	enemy_model_ = Model::CreateFromOBJ("enemy");
	// 02_09 10枚目 敵位置決めて敵クラス初期化 → 02_10の5枚目で削除
	//	Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(14, 18);
	// enemy_->Initialize(enemy_model_, &camera_, enemyPosition);

	// 02_10 5枚目（for文の中身全部）
	for (int32_t i = 0; i < 2; ++i) {
		Enemy* newEnemy = new Enemy();

		Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(30 + i * 2, 18);

		newEnemy->Initialize(enemy_model_, &camera_, enemyPosition);

		// ★ マップチップフィールドを渡す
		newEnemy->SetMapChipField(mapChipField_);

		// ★ GameScene を渡すのはそのまま残す
		newEnemy->SetGameScene(this);

		enemies_.push_back(newEnemy);
	}

	// 02_11_16枚目 モデル読み込み
	deathParticle_model_ = Model::CreateFromOBJ("deathParticle");

	// 02_16
	particle_model_ = Model::CreateFromOBJ("particle");

	// 02_11_16枚目 仮の生成処理 後で消す
	// 02_12 13枚目で消す
	//	deathParticles_ = new DeathParticles;
	//	deathParticles_->Initialize
	//	    (deathParticle_model_, &camera_, playerPosition);

	// 02_12_4枚目 ゲームプレイフェーズから開始
	//	phase_ = Phase::kPlay;
	// ↑を02_13_27枚目で変更
	phase_ = Phase::kFadeIn;

	// 02_13 27枚目
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	HitEffect::SetModel(particle_model_);
	HitEffect::SetCamera(&camera_);
}

// 02_12 10枚目 GameScene::Update関数で呼び出しておく
// player->draw();をif(!player_->IsDead()){}で囲む
void GameScene::ChangePhase() {

	switch (phase_) {
	case Phase::kPlay:
		// 02_12 13枚目 if文から中身まで全部実装
		// Initialize関数のいきなりパーティクル発生処理は消す
		if (player_->IsDead()) {
			// 死亡演出
			phase_ = Phase::kDeath;

			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			deathParticles_ = new DeathParticles;
			deathParticles_->Initialize(deathParticle_model_, &camera_, deathParticlesPosition);
		} else if (AreAllEnemiesDead() && hitEffects_.empty()) {
			// 敵が全滅してHitEffectも消えたらゲームクリア
			phase_ = Phase::kGameClear;
			fade_->Start(Fade::Status::FadeOut, 1.0f);
		}
		break;
	case Phase::kDeath:

		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}

		break;
	case Phase::kGameClear:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;

	case Phase::kFadeIn:

	case Phase::kFadeOut:
	default:
		fade_->Update();
		break;
	}
}

// 敵が全員倒されているかチェック
bool GameScene::AreAllEnemiesDead() const {
	for (Enemy* enemy : enemies_) {
		if (!enemy->IsDead())
			return false;
	}
	return true;
}


void GameScene::GenerateBlocks() {

	uint32_t numBlockVirtical = mapChipField_->GetNumBlockVirtical();
	uint32_t numBlockHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numBlockVirtical);
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {
		worldTransformBlocks_[i].resize(numBlockHorizontal);
	}

	// ブロックの生成
	for (uint32_t i = 0; i < numBlockVirtical; ++i) {

		for (uint32_t j = 0; j < numBlockHorizontal; ++j) {

			if (mapChipField_->GetMapChipTypeByIndex(j, i) == MapChipType::kBlock) {
				WorldTransform* worldTransform = new WorldTransform();
				worldTransform->Initialize();
				worldTransformBlocks_[i][j] = worldTransform;
				worldTransformBlocks_[i][j]->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);
			}
		}
	}
}

// ゲームシーン更新
void GameScene::Update() {


	// リトライキー（例: Rキー）でリトライ
	if (Input::GetInstance()->TriggerKey(DIK_R)) {
		Retry();
	}

	 // ポーズトグル（例: Pキーで切り替え）
	if (Input::GetInstance()->TriggerKey(DIK_P)) {
		if (phase_ == Phase::kPlay) {
			phase_ = Phase::kPause;
		} else if (phase_ == Phase::kPause) {
			phase_ = Phase::kPlay;
		}
	}

	// デスフラグの立ったエフェクトを削除
	hitEffects_.remove_if([](HitEffect* hitEffect) {
		if (hitEffect->IsDead()) {
			delete hitEffect;

			return true;
		}
		return false;
	});

	// 02_15 7枚目 デスフラグの立った敵を削除
	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->IsDead()) {
			delete enemy;
			return true;
		}
		return false;
	});

	ChangePhase();

	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		if (fade_->IsFinished()) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kPlay;
		}

		skydome_->Update();
		CController_->Update();
		//		worldTransformSkydome_.UpdateMatrix();
		//		cameraController->Update();

		// 自キャラの更新
		player_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		// UpdateCamera();
		/*
		#ifdef _DEBUG
		        if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		            // フラグをトグル
		            isDebugCameraActive_ = !isDebugCameraActive_;
		        }
		#endif
		*/
		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		// UpdateBlocks();
		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {

				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}
		break;
	case Phase::kPlay:
		skydome_->Update();
		CController_->Update();
		//		worldTransformSkydome_.UpdateMatrix();
		//		cameraController->Update();

		// 自キャラの更新
		player_->Update();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		//		UpdateCamera();
		/*
		#ifdef _DEBUG
		        if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		            // フラグをトグル
		            isDebugCameraActive_ = !isDebugCameraActive_;
		        }
		#endif
		*/
		// カメラの処理
		if (isDebugCameraActive_) {
			debugCamera_->Update();
			camera_.matView = debugCamera_->GetCamera().matView;
			camera_.matProjection = debugCamera_->GetCamera().matProjection;
			// ビュープロジェクション行列の転送
			camera_.TransferMatrix();
		} else {
			// ビュープロジェクション行列の更新と転送
			camera_.UpdateMatrix();
		}

		//		UpdateBlocks();
		// ブロックの更新
		for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
			for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {

				if (!worldTransformBlock)
					continue;

				// アフィン変換～DirectXに転送
				WorldTransformUpdate(*worldTransformBlock);
			}
		}

		CheckAllCollisions();

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}
		break;

	case Phase::kGameClear:

		if (!gameClearScene_) {
			gameClearScene_ = new GameClearScene();
			gameClearScene_->Initialize();
		}
		gameClearScene_->Update();

		if (gameClearScene_->IsFinished()) {
			finished_ = true;
		}
		break;

	case Phase::kDeath:
		if (deathParticles_ && deathParticles_->IsFinished()) {
			phase_ = Phase::kFadeOut;
		}

		skydome_->Update();
		CController_->Update();
		//		worldTransformSkydome_.UpdateMatrix();
		//		UpdateCamera();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		if (deathParticles_) {
			deathParticles_->Update();
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}
		break;
	case Phase::kFadeOut:
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}

		skydome_->Update();
		CController_->Update();
		//		worldTransformSkydome_.UpdateMatrix();
		//		UpdateCamera();

		for (Enemy* enemy : enemies_) {
			enemy->Update();
		}

		for (HitEffect* hitEffect : hitEffects_) {
			hitEffect->Update();
		}

		break;
	}
}

// -------------------------------------------------------------
// 全敵倒したらゲームクリアにする判定
// -------------------------------------------------------------
void GameScene::CheckGameClear() {
	if (enemies_.empty()) {
		// 敵が全滅したらフェーズをクリアに
		phase_ = Phase::kGameClear;
		if (fade_) {
			fade_->Start(Fade::Status::FadeOut, 1.0f); // 任意：フェードアウト
		}
	}
}

void GameScene::Draw() {

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3Dオブジェクト描画前処理
	Model::PreDraw(dxCommon->GetCommandList());

	// 自キャラの描画
	if (!player_->IsDead())
		player_->Draw();

	// 天球描画
	skydome_->Draw();

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform*& worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;

			block_model_->Draw(*worldTransformBlock, camera_);
		}
	}

	// 02_09 12枚目 敵更新 → 02_10 7枚目で更新
	//	enemy_->Draw();
	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	// 02_11 18枚目 デスパーティクルあれば描画
	if (deathParticles_) {
		deathParticles_->Draw();
	}

	for (HitEffect* hitEffect : hitEffects_) {
		hitEffect->Draw();
	}

	Model::PostDraw();

	// スプライト描画前処理
	Sprite::PreDraw(dxCommon->GetCommandList());

	// スプライト描画後処理
	Sprite::PostDraw();

	// 02_13 28枚目
	fade_->Draw();
}

// 02_10 16枚目
void GameScene::CheckAllCollisions() {

	// 判定対象1と2の座標
	AABB aabb1, aabb2;

#pragma region 自キャラと敵キャラの当たり判定
	{
		// 自キャラの座標
		aabb1 = player_->GetAABB();

		// 自キャラと敵弾全ての当たり判定
		for (Enemy* enemy : enemies_) {

			// コリジョン無効の敵はスキップ
			if (enemy->IsCollisionDisabled())
				continue;

			// 敵弾の座標
			aabb2 = enemy->GetAABB();

			// AABB同士の交差判定
			if (IsCollision(aabb1, aabb2)) {
				// 自キャラの衝突時コールバックを呼び出す
				player_->OnCollision(enemy);
				// 敵弾の衝突時コールバックを呼び出す
				enemy->OnCollision(player_);
			}
		}
	}
#pragma endregion
}

bool GameScene::IsCleared() const {
    return phase_ == Phase::kGameClear; }

void GameScene::Retry() {
	// 既存の動的メモリを破棄
	delete player_;
	player_ = nullptr;

	for (Enemy* enemy : enemies_)
		delete enemy;
	enemies_.clear();

	for (auto& line : worldTransformBlocks_) {
		for (WorldTransform* block : line)
			delete block;
	}
	worldTransformBlocks_.clear();

	for (HitEffect* effect : hitEffects_)
		delete effect;
	hitEffects_.clear();

	delete deathParticles_;
	deathParticles_ = nullptr;

	delete fade_;
	fade_ = nullptr;

	delete skydome_;
	skydome_ = nullptr;

	// 必要なら camera やモデルは残してもよい
	// 初期化を再度呼ぶ
	Initialize();
}
