#include "GameScene.h"
#include "Math.h"
#include <cstdlib>
#include <ctime>

void GameScene::Initialize(GameManager* manager) {
	manager_ = manager;

	playerModel_ = Model::CreateFromOBJ("player");
	enemyModel_ = Model::CreateFromOBJ("enemy");

	uint32_t lockonTexture = TextureManager::Load("lockon_br.png");

	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();

	std::srand((unsigned)std::time(nullptr));

	enemies_.clear();

	debugCamera_ = new DebugCamera(1280, 720);

	player_ = new Player();
	player_->Initialize(playerModel_, &camera_, {0, 0, 0}, lockonTexture);
	player_->SetEnemies(&enemies_);
	player_->SetAimPlaneZ(10.0f);

	// WaveManager開始
	waveManager_.Initialize();
	currentWaveIndex_ = 0;
	waveManager_.StartWave(currentWaveIndex_);

	wavePhase_ = WavePhase::Fighting;
	waveWaitTimer_ = 0.0f;

	// Pause初期化
	isPaused_ = false;
	pause_.Initialize(&camera_); // ★これが必須
	pause_.Close();
}

void GameScene::Finalize() { Cleanup(); }

void GameScene::Cleanup() {
	for (Enemy* e : enemies_)
		delete e;
	enemies_.clear();

	delete player_;
	player_ = nullptr;

	delete debugCamera_;
	debugCamera_ = nullptr;

	delete playerModel_;
	playerModel_ = nullptr;

	delete enemyModel_;
	enemyModel_ = nullptr;
}

void GameScene::Update() {
	const float dt = 1.0f / 60.0f;

	// ====== ポーズ切り替え（ESC） ======
	if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		isPaused_ = !isPaused_;
		if (isPaused_)
			pause_.Open();
		else
			pause_.Close();
	}

	// ====== ポーズ中は「ゲーム更新しない」 ======
	if (isPaused_) {
		UpdatePause();
		return;
	}

	// ====== 通常更新 ======
	player_->Update();

	UpdateEnemies();
	ResolveCollisions();
	RemoveDeadEnemies();
	UpdateWaves(dt);

	// ★ここで GameOver 判定（例：プレイヤーHPが0なら）
	// Playerに GetHP() が無いなら、まずそれを作るのが正解。
	// ここでは仮に IsDead() がある想定で書く：
	// if (player_->IsDead()) { manager_->RequestChangeScene(SceneType::GameOver); }
	// ★ゲームオーバー判定
	if (player_ && player_->IsDead()) {
		manager_->RequestChangeScene(SceneType::GameOver);
		return;
	}
}

void GameScene::UpdatePause() {
	PauseMenu::Result r = pause_.Update();

	if (r == PauseMenu::Result::None)
		return;

	if (r == PauseMenu::Result::Resume) {
		isPaused_ = false;
		pause_.Close();
		return;
	}

	if (r == PauseMenu::Result::Restart) {
		// シーンを作り直す＝完全リスタート
		manager_->RequestChangeScene(SceneType::Game);
		return;
	}

	if (r == PauseMenu::Result::ToTitle) {
		manager_->RequestChangeScene(SceneType::Title);
		return;
	}
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3D
	Model::PreDraw(dxCommon->GetCommandList());
	if (player_)
		player_->Draw3D();
	for (Enemy* e : enemies_)
		if (e)
			e->Draw3D();

	// ★ポーズメニューもModel描画中に描く
	if (isPaused_) {
		pause_.Draw();
	}

	Model::PostDraw();

	// 2D（ロックオンUIなど）
	Sprite::PreDraw(dxCommon->GetCommandList());
	if (player_)
		player_->Draw2D();
	Sprite::PostDraw();
}


void GameScene::UpdateEnemies() {
	for (Enemy* e : enemies_) {
		if (!e)
			continue;

		e->Update(player_->GetWorldPosition());

		if (!e->IsDead() && e->CheckCollision(player_->GetWorldPosition(), player_->GetRadius())) {
			player_->OnHitByBeam();
			e->Kill();
		}
	}
}

void GameScene::ResolveCollisions() {
	// Shooterの弾 vs プレイヤー
	for (Enemy* e : enemies_) {
		ShooterEnemy* shooter = dynamic_cast<ShooterEnemy*>(e);
		if (!shooter)
			continue;

		for (EnemyBullet* b : shooter->GetBullets()) {
			if (!b || b->IsDead())
				continue;

			if (HitSphere(b->GetWorldPosition(), b->GetRadius(), player_->GetWorldPosition(), player_->GetRadius())) {
				player_->OnHitByBeam();
				b->Kill();
			}
		}
	}

	// プレイヤー弾 vs 敵
	for (Enemy* e : enemies_) {
		if (!e || e->IsDead())
			continue;

		for (PlayerBullet* b : player_->GetBullets()) {
			if (!b || b->IsDead())
				continue;

			if (HitSphere(b->GetWorldPosition(), b->GetRadius(), e->GetWorldPosition(), e->GetRadius())) {
				b->OnCollision();
				e->Damage(1);
			}
		}
	}

	// ミサイル vs 敵
	for (Enemy* e : enemies_) {
		if (!e || e->IsDead())
			continue;

		for (HomingArcBullet* m : player_->GetArcBullets()) {
			if (!m || m->IsDead())
				continue;

			if (HitSphere(m->GetWorldPosition(), m->GetRadius(), e->GetWorldPosition(), e->GetRadius())) {
				m->OnCollision();
				e->Damage(3);
			}
		}
	}

	// Turret弾 vs プレイヤー
	for (Enemy* e : enemies_) {
		BarrageTurretEnemy* turret = dynamic_cast<BarrageTurretEnemy*>(e);
		if (!turret)
			continue;

		for (EnemyBullet* b : turret->GetBullets()) {
			if (!b || b->IsDead())
				continue;

			if (HitSphere(b->GetWorldPosition(), b->GetRadius(), player_->GetWorldPosition(), player_->GetRadius())) {
				player_->OnHitByBeam();
				b->Kill();
			}
		}
	}
}

void GameScene::RemoveDeadEnemies() {
	for (auto it = enemies_.begin(); it != enemies_.end();) {
		Enemy* e = *it;
		if (e && e->IsDead()) {
			delete e;
			it = enemies_.erase(it);
		} else {
			++it;
		}
	}
}

void GameScene::UpdateWaves(float dt) {
	if (wavePhase_ == WavePhase::Clear) {
		// ★ここでクリア遷移
		manager_->RequestChangeScene(SceneType::Clear);
		return;
	}

	if (wavePhase_ == WavePhase::Fighting) {
		waveManager_.Update(dt, *this, (int)enemies_.size());

		if (waveManager_.IsFinished()) {
			wavePhase_ = WavePhase::Waiting;
			waveWaitTimer_ = 1.0f;
		}
		return;
	}

	// Waiting
	waveWaitTimer_ -= dt;
	if (waveWaitTimer_ > 0.0f)
		return;

	currentWaveIndex_++;

	if (currentWaveIndex_ >= waveManager_.GetWaveCount()) {
		wavePhase_ = WavePhase::Clear;
		return;
	}

	waveManager_.StartWave(currentWaveIndex_);
	wavePhase_ = WavePhase::Fighting;
}

// ===== Spawn =====
void GameScene::SpawnTacklerEnemy(const Vector3& pos) {
	Enemy* e = new TacklerEnemy();
	e->Initialize(enemyModel_, &camera_, pos);
	enemies_.push_back(e);
}

void GameScene::SpawnShooterEnemy(const Vector3& pos) {
	Enemy* e = new ShooterEnemy();
	e->Initialize(enemyModel_, &camera_, pos);
	enemies_.push_back(e);
}

void GameScene::SpawnTurretEnemy(const Vector3& pos) {
	Enemy* e = new BarrageTurretEnemy();
	e->Initialize(enemyModel_, &camera_, pos);
	enemies_.push_back(e);
}

bool GameScene::HitSphere(const Vector3& aPos, float aR, const Vector3& bPos, float bR) {
	Vector3 d = aPos - bPos;
	float dist = Length(d);
	return dist < (aR + bR);
}
