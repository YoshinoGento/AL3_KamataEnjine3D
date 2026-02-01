#include "GameScene.h"
#include "Math.h"
#include <cstdlib>
#include <ctime>

void GameScene::Initialize(GameManager* manager) {
	manager_ = manager;

	playerModel_ = Model::CreateFromOBJ("player1");

	// ★種類ごとに別OBJを読む
	tacklerModel_ = Model::CreateFromOBJ("enemy_tackler");
	shooterModel_ = Model::CreateFromOBJ("enemy_shooter");
	turretModel_ = Model::CreateFromOBJ("enemy_turret");

	// ★（任意）弾も別OBJ
	enemyBulletModel_ = Model::CreateFromOBJ("BossMissile");
	playerBulletModel_ = Model::CreateFromOBJ("PlayerBullet");
	playerMissile_ = Model::CreateFromOBJ("PlayerMissile");

	sky_.Initialize("FaceSkySphere", 3.0f);

	SpawnDirector::Settings spawn;
	spawn.minEnemyDist = 3.0f;
	spawn.minPlayerDist = 12.0f;
	spawn.spawnMinR = 15.0f;
	spawn.spawnMaxR = 30.0f;
	spawn.maxRetry = 50;

	spawn.nearSearchRadius = 6.0f;
	spawn.nearSearchTry = 30;

	spawner_.Initialize(spawn);

	uint32_t lockonTexture = TextureManager::Load("lockon_br.png");

	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();

	std::srand((unsigned)std::time(nullptr));
	enemies_.clear();

	debugCamera_ = new DebugCamera(1280, 720);

	player_ = new Player();
	player_->Initialize(playerModel_, &camera_, {0, 0, 0}, lockonTexture);
	player_->SetBulletModel(playerBulletModel_);
	player_->SetMissileModel(playerMissile_);
	player_->SetEnemies(&enemies_);
	player_->SetAimPlaneZ(10.0f);

	waveManager_.Initialize();
	currentWaveIndex_ = 0;
	waveManager_.StartWave(currentWaveIndex_);

	wavePhase_ = WavePhase::Fighting;
	waveWaitTimer_ = 0.0f;

	// ===== 音：ロード =====
	Audio* audio = Audio::GetInstance();

	bgmGame_ = audio->LoadWave("game_bgm.wav");
	seShot_ = audio->LoadWave("shot.wav");
	seHit_ = audio->LoadWave("hit.wav");
	sePauseOpen_ = audio->LoadWave("pause.wav");

	// ===== 音：BGM再生（ループ）=====
	bgmVoice_ = audio->PlayWave(bgmGame_, true, 0.03f); // 0.3は好みでOK

	player_->SetShotSE(seShot_);
	player_->SetMissileSE(seShot_); // ミサイルも同じ音でいいなら一旦これ

	isPaused_ = false;
	pause_.Initialize(&camera_);
	pause_.Close();
}

void GameScene::Finalize() {
	sky_.Finalize(); // ★追加
	Cleanup();
}

void GameScene::Cleanup() {

	// ===== 音：BGM停止 =====
	// ★BGM停止（シーンを抜けるときに必ず止める）
	Audio* audio = Audio::GetInstance();
	audio->StopWave(bgmVoice_);
	bgmVoice_ = 0;

	for (Enemy* e : enemies_)
		delete e;
	enemies_.clear();

	delete player_;
	player_ = nullptr;
	delete debugCamera_;
	debugCamera_ = nullptr;

	delete playerModel_;
	playerModel_ = nullptr;

	// ★追加：敵モデル全部解放
	delete tacklerModel_;
	tacklerModel_ = nullptr;
	delete shooterModel_;
	shooterModel_ = nullptr;
	delete turretModel_;
	turretModel_ = nullptr;

	// ★追加：弾モデル（任意）
	delete enemyBulletModel_;
	enemyBulletModel_ = nullptr;
}

void GameScene::ApplyEnemySeparation(float dt) {

	// ==== 調整しやすいパラメータ ====
	const float desiredDist = 3.0f; // これより近いと押し戻す
	const float pushPower = 10.0f;  // 基本の強さ（大きいほど散る）

	// タイプ別倍率（ここが肝）
	const float kTacklerMul = 0.25f; // タックラーは弱め
	const float kTurretMul = 0.15f;  // 砲台はもっと弱め

	const int n = (int)enemies_.size();
	if (n <= 1)
		return;

	for (int i = 0; i < n; ++i) {
		Enemy* a = enemies_[i];
		if (!a || a->IsDead())
			continue;

		// --- 型判定 ---
		TacklerEnemy* tacklerA = dynamic_cast<TacklerEnemy*>(a);
		BarrageTurretEnemy* turretA = dynamic_cast<BarrageTurretEnemy*>(a);

		// Shooterは倍率1.0なので判定不要
		float mulA = 1.0f;
		if (tacklerA)
			mulA = kTacklerMul;
		if (turretA)
			mulA = kTurretMul;

		Vector3 aPos = a->GetWorldPosition();
		Vector3 push{0, 0, 0};

		for (int j = 0; j < n; ++j) {
			if (i == j)
				continue;

			Enemy* b = enemies_[j];
			if (!b || b->IsDead())
				continue;

			Vector3 bPos = b->GetWorldPosition();
			Vector3 diff = aPos - bPos;

			float dist = Length(diff);
			if (dist < 1e-5f) {
				diff = {0.01f * float(i + 1), 0.0f, 0.01f * float(j + 1)};
				dist = Length(diff);
			}

			if (dist < desiredDist) {
				float t = 1.0f - (dist / desiredDist);
				push += Normalized(diff) * (t * t);
			}
		}

		if (Length(push) < 1e-5f)
			continue;

		Vector3 delta = push * (pushPower * mulA * dt);

		// Z方向は崩れやすいのでカット
		delta.z = 0.0f;

		Vector3 newPos = aPos + delta;

		// 砲台はZ固定絶対
		if (turretA)
			newPos.z = aPos.z;

		a->SetWorldPosition(newPos);
	}
}

void GameScene::Update() {
	const float dt = 1.0f / 60.0f;

	sky_.Update(camera_);

	// ====== ポーズ切り替え（ESC） ======
	if (Input::GetInstance()->TriggerKey(DIK_ESCAPE)) {
		isPaused_ = !isPaused_;

		if (isPaused_) {
			pause_.Open();

			// ★BGMを一時停止（0でも有効なので条件分岐しない）
			Audio::GetInstance()->PauseWave(bgmVoice_);

			Audio::GetInstance()->PlayWave(sePauseOpen_, false, 0.01f);

		} else {
			pause_.Close();

			// ★BGMを再開
			Audio::GetInstance()->ResumeWave(bgmVoice_);
		}
	}

	// ====== ポーズ中は「ゲーム更新しない」 ======
	if (isPaused_) {
		UpdatePause();
		return;
	}

	// ====== 通常更新 ======
	player_->Update();

	UpdateEnemies();
	ApplyEnemySeparation(dt);
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
		Audio::GetInstance()->StopWave(bgmVoice_);
		manager_->RequestChangeScene(SceneType::Game);
		return;
	}

	if (r == PauseMenu::Result::ToTitle) {
		Audio::GetInstance()->StopWave(bgmVoice_);
		manager_->RequestChangeScene(SceneType::Title);
		return;
	}
}

void GameScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// 3D
	Model::PreDraw(dxCommon->GetCommandList());
	sky_.Draw(camera_); // ★天球
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

Vector3 GameScene::GetSafeSpawnPos() {
	Vector3 playerPos = player_->GetWorldPosition();
	return spawner_.GetSpawnPosRandomRing(playerPos, enemies_);
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

	Vector3 playerPos = player_->GetWorldPosition();

	Vector3 finalPos{};
	if (!spawner_.FindNearestSafePos(pos, playerPos, enemies_, finalPos)) {
		// pos付近が無理ならリングスポーン
		finalPos = spawner_.GetSpawnPosRandomRing(playerPos, enemies_);
	}

	Enemy* enemy = new TacklerEnemy();
	enemy->Initialize(tacklerModel_, &camera_, finalPos);
	enemies_.push_back(enemy);
}

void GameScene::SpawnShooterEnemy(const Vector3& pos) {

	Vector3 playerPos = player_->GetWorldPosition();

	// 台本pos → 安全にズラす
	Vector3 finalPos{};
	if (!spawner_.FindNearestSafePos(pos, playerPos, enemies_, finalPos)) {
		finalPos = spawner_.GetSpawnPosRandomRing(playerPos, enemies_);
	}

	ShooterEnemy* enemy = new ShooterEnemy();
	enemy->Initialize(shooterModel_, &camera_, finalPos);

	// ★超重要：Shooterの弾モデルを設定する
	enemy->SetBulletModel(enemyBulletModel_);

	enemies_.push_back(enemy);
}

void GameScene::SpawnTurretEnemy(const Vector3& pos) {

	Vector3 playerPos = player_->GetWorldPosition();

	// ★砲台は最低距離を強めにする（おすすめ）
	const float turretMinDist = 300.0f;

	Vector3 finalPos{};
	bool ok = spawner_.FindNearestSafePos(pos, playerPos, enemies_, finalPos);

	// 近すぎたら台本位置は不採用にする
	if (ok) {
		float d = Length(finalPos - playerPos);
		if (d < turretMinDist) {
			ok = false;
		}
	}

	if (!ok) {
		// 砲台は遠距離に出したいのでリングへ
		finalPos = spawner_.GetSpawnPosFrontRing(playerPos, enemies_);
	}

	BarrageTurretEnemy* enemy = new BarrageTurretEnemy();
	enemy->Initialize(turretModel_, &camera_, finalPos);

	// ★超重要：Turretの弾モデルを設定する
	enemy->SetBulletModel(enemyBulletModel_);

	enemies_.push_back(enemy);
}

bool GameScene::HitSphere(const Vector3& aPos, float aR, const Vector3& bPos, float bR) {
	Vector3 d = aPos - bPos;
	float dist = Length(d);
	return dist < (aR + bR);
}