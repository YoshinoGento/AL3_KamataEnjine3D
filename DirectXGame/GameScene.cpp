#include "GameScene.h"
#include "KamataEngine.h"
#include "Math.h"
#include <cstdlib>
#include <ctime>
#include "BarrgeTurretEnemy.h"

using namespace KamataEngine;

void GameScene::Initialize() {
	player_model_ = Model::CreateFromOBJ("player");
	enemy_model_ = Model::CreateFromOBJ("enemy");

	uint32_t lockonTexture = TextureManager::Load("lockon_br.png");
;

	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();

	// 乱数
	std::srand((unsigned)std::time(nullptr));

	// Z平面（敵が出るZ）
	const float z = 10.0f;

	// 敵リストは一旦空に
	enemies_.clear();

	// デバッグカメラ
	debugCamera_ = new DebugCamera(1280, 720);

	// プレイヤー生成
	player_ = new Player();
	player_->Initialize(player_model_, &camera_, {0, 0, 0}, lockonTexture);

	// ロックオン用に敵リストを渡す（このポインタはずっと同じでOK）
	player_->SetEnemies(&enemies_);
	player_->SetAimPlaneZ(z);

    // ----------------------
	// WaveManager 初期化（ここが本体）
	// ----------------------
	waveManager_.Initialize();
	currentWave_ = 0;
	waveManager_.StartWave(currentWave_);

	// Wave間の待ちを入れるなら（あなたの変数名に合わせて）
	wavePhase_ = WavePhase::Fighting;
	waveWaitTimer_ = 0.0f;
}


void GameScene::Update() {
	player_->Update();

	// ======================
	// 1) 更新フェーズ（消さない）
	// ======================
	for (Enemy* enemy : enemies_) {
		if (!enemy) {
			continue;
		}
		enemy->Update(player_->GetWorldPosition());

		// 体当たり判定（ここでKillフラグだけ立てる）
		if (!enemy->IsDead() && enemy->CheckCollision(player_->GetWorldPosition(), player_->GetRadius())) {
			player_->OnHitByBeam();
			enemy->Kill(); // ★消すのは後
		}
	}

	// ======================
	// 2) 当たり判定フェーズ（B）
	// ======================

	// --- 敵弾 vs プレイヤー ---
	for (Enemy* enemy : enemies_) {
		ShooterEnemy* shooter = dynamic_cast<ShooterEnemy*>(enemy);
		if (!shooter) {
			continue;
		}

		for (EnemyBullet* bullet : shooter->GetBullets()) {
			if (!bullet || bullet->IsDead()) {
				continue;
			}

			if (HitSphere(bullet->GetWorldPosition(), bullet->GetRadius(), player_->GetWorldPosition(), player_->GetRadius())) {
				player_->OnHitByBeam();
				bullet->OnCollision(); // ★Kill()が無いならこれにする
			}
		}
	}

	// --- プレイヤー弾 vs 敵 ---
	for (Enemy* enemy : enemies_) {
		if (!enemy || enemy->IsDead()) {
			continue;
		}

		for (PlayerBullet* bullet : player_->GetBullets()) {
			if (!bullet || bullet->IsDead()) {
				continue;
			}

			if (HitSphere(bullet->GetWorldPosition(), bullet->GetRadius(), enemy->GetWorldPosition(), enemy->GetRadius())) {
				bullet->OnCollision();
				enemy->Damage(1);
			}
		}
	}

	// --- ミサイル vs 敵 ---
	for (Enemy* enemy : enemies_) {
		if (!enemy || enemy->IsDead()) {
			continue;
		}

		for (HomingArcBullet* m : player_->GetArcBullets()) {
			if (!m || m->IsDead()) {
				continue;
			}

			if (HitSphere(m->GetWorldPosition(), m->GetRadius(), enemy->GetWorldPosition(), enemy->GetRadius())) {
				m->OnCollision();
				enemy->Damage(3);
			}
		}
	}

	// ======================
	// 3) 削除フェーズ（A：delete&erase）
	// ======================
	for (auto it = enemies_.begin(); it != enemies_.end();) {
		Enemy* enemy = *it;
		if (enemy && enemy->IsDead()) {
			delete enemy;
			it = enemies_.erase(it);
		} else {
			++it;
		}
	}

	// ② Wave進行（WaveManagerに統一）
	const float dt = 1.0f / 60.0f;

	if (wavePhase_ == WavePhase::Clear) {
		// ここでステージクリア演出やシーン遷移をしたいなら書く
	} else if (wavePhase_ == WavePhase::Fighting) {
		waveManager_.Update(dt, *this, (int)enemies_.size());

		if (waveManager_.IsFinished()) {
			wavePhase_ = WavePhase::Waiting;
			waveWaitTimer_ = 1.0f; // 次Waveまでの“間”
		}
	} else if (wavePhase_ == WavePhase::Waiting) {
		waveWaitTimer_ -= dt;

		if (waveWaitTimer_ <= 0.0f) {
			currentWave_++;

			// ★ここが追加：Waveがもう無いならClearへ
			if (currentWave_ >= waveManager_.GetWaveCount()) {
				wavePhase_ = WavePhase::Clear;
				// 例：ここでBGM止める/クリアUI出す/シーン切替など
				// gameManager_->ChangeScene(CLEAR); みたいなのを呼ぶならここ
			} else {
				waveManager_.StartWave(currentWave_);
				wavePhase_ = WavePhase::Fighting;
			}
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

void GameScene::SpawnTacklerEnemy(const Vector3& pos) {
	Enemy* enemy = new TacklerEnemy();
	enemy->Initialize(enemy_model_, &camera_, pos);

	enemy->FaceTo(player_->GetWorldPosition()); // ★追加

	enemies_.push_back(enemy);
}

void GameScene::SpawnShooterEnemy(const Vector3& pos) {
	Enemy* enemy = new ShooterEnemy();
	enemy->Initialize(enemy_model_, &camera_, pos);

	enemy->FaceTo(player_->GetWorldPosition());

	enemies_.push_back(enemy);
}


void GameScene::SpawnEnemy() {
	float x = (float(rand()) / RAND_MAX) * 12.0f - 6.0f; // -6〜+6
	Vector3 pos = {x, 0.0f, 10.0f};

	Enemy* enemy_Tackler = new TacklerEnemy();
	enemy_Tackler->Initialize(enemy_model_, &camera_, pos);
	enemies_.push_back(enemy_Tackler);
}

void GameScene::SpawnTurretEnemy(const Vector3& pos) {
	Enemy* enemy = new BarrageTurretEnemy();
	enemy->Initialize(enemy_model_, &camera_, pos);

	enemy->FaceTo(player_->GetWorldPosition());
	enemies_.push_back(enemy);
}




bool GameScene::HitSphere(const Vector3& aPos, float aR, const Vector3& bPos, float bR) {

	Vector3 d = aPos - bPos;
	float dist = Length(d);
	return dist < (aR + bR);
}

int GameScene::CountTacklers(const std::vector<Enemy*>& enemies) { 

 int count = 0;
	for (Enemy* enemy : enemies) {
		if (dynamic_cast<TacklerEnemy*>(enemy)) {
			++count;
		}
	}
	return count;

}

int GameScene::CountShooters(const std::vector<Enemy*>& enemies) { 

	 int count = 0;
	for (Enemy* enemy : enemies) {
		if (dynamic_cast<ShooterEnemy*>(enemy)) {
			++count;
		}
	}
	return count;
}



Vector3 GameScene::RandomSpawnPos() const {
	float x = (float(rand()) / RAND_MAX) * (respawnXRange_ * 2.0f) - respawnXRange_;
	float y = 0.0f;      // 必要ならランダムYも
	float z = respawnZ_; // 君の respawnZ_ を使う
	return {x, y, z};
}

bool GameScene::AreAllEnemiesDead() const {
	for (Enemy* e : enemies_) {
		if (e && !e->IsDead()) { // IsDead() が無ければ hp_<=0 などに置換
			return false;
		}
	}
	return true;
}

void GameScene::SpawnWave(const WaveSpawn& waveSpawn) {
	for (int i = 0; i < waveSpawn.tackler; ++i) {
		SpawnTacklerEnemy(RandomSpawnPos());
	}
	for (int i = 0; i < waveSpawn.shooter; ++i) {
		SpawnShooterEnemy(RandomSpawnPos());
	}
	for (int i = 0; i < waveSpawn.turret; ++i) {
		SpawnTurretEnemy(RandomSpawnPos());
	}
}


void GameScene::InitWaves() {}

void GameScene::UpdateWaves() {
	if (waveState_ == WaveState::Clear)
		return;

	if (waveState_ == WaveState::Fighting) {
		if (AreAllEnemiesDead()) {
			waveState_ = WaveState::WaitingNext;
			waveWaitTimer_ = 1.0f; // “間”を作る（ここが演出の核）
		}
		return;
	}

	// WaitingNext
	waveWaitTimer_ -= 1.0f / 60.0f; // deltaTimeがあるなら差し替え
	if (waveWaitTimer_ > 0.0f)
		return;

	waveIndex_++;
	if (waveIndex_ >= (int)waves_.size()) {
		waveState_ = WaveState::Clear;
		// ここでステージクリア演出へ
		return;
	}

	SpawnWave(waves_[waveIndex_]);
	waveState_ = WaveState::Fighting;
}
