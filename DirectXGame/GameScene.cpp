#include "GameScene.h"
#include "KamataEngine.h"
#include "Math.h"
#include <cstdlib>
#include <ctime>

using namespace KamataEngine;

void GameScene::Initialize() {
	player_model_ = Model::CreateFromOBJ("player");
	enemy_model_ = Model::CreateFromOBJ("enemy");

	uint32_t lockonTexture = TextureManager::Load("lockon_br.png");

	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();

	enemies_.clear();

	const float z = 10.0f;

	// ★3体ずつ出す
	for (int i = 0; i < 3; ++i) {

		// ---- 突進敵 ----
		Enemy* enemyTackler = new TacklerEnemy();
		Vector3 positionTackler = {float(i * 3 - 6), 0.0f, z};
		enemyTackler->Initialize(enemy_model_, &camera_, positionTackler);
		enemies_.push_back(enemyTackler);

		// ---- 射撃敵 ----
		Enemy* enemyShooter = new ShooterEnemy();
		Vector3 positionShooter = {float(i * 3 + 2), 1.0f, z};             // +2 で右へ
		enemyShooter->Initialize(enemy_model_, &camera_, positionShooter); // ★ここが重要
		enemies_.push_back(enemyShooter);
	}

	debugCamera_ = new DebugCamera(1280, 720);

	player_ = new Player();
	player_->Initialize(player_model_, &camera_, {0, 0, 0}, lockonTexture);

	player_->SetEnemies(&enemies_);
	player_->SetAimPlaneZ(z);

	std::srand((unsigned)std::time(nullptr));


	////----ウェーブ設定----////
	waves_.clear();

	// 演出が出る順に波を作る
	waves_.push_back({3, 0, 0}); // Wave1: 突進3（チュートリアル）
	waves_.push_back({0, 3, 0}); // Wave2: 射撃3（回避練習）
	waves_.push_back({2, 2, 0}); // Wave3: 混合（本番）
	waves_.push_back({3, 3, 0}); // Wave4: ラッシュ

	waveIndex_ = 0;
	waveState_ = WaveState::Fighting;

	///----------------------////
}



void GameScene::Update() {
#ifdef _DEBUG
	// if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
	//     isDebugCameraActive_ = !isDebugCameraActive_;
	// }
#endif

	player_->Update();

	const float deltaTime = 1.0f / 60.0f; // 仮（今の環境に合わせてOK）

	// --- 敵の更新＆削除 ---
	for (auto it = enemies_.begin(); it != enemies_.end();) {
		Enemy* enemy = *it;

		enemy->Update(player_->GetWorldPosition());

		if (enemy->CheckCollision(player_->GetWorldPosition(), player_->GetRadius())) {
			player_->OnHitByBeam();
			enemy->Kill(); // ← 当たったら自爆タックル
		}

		if (enemy->IsDead()) {
			delete enemy;
			it = enemies_.erase(it);

			// ★ 敵が減ったのでリスポーンタイマー開始
			respawnTimer_ = respawnInterval_;
			continue;
		}

		++it;
	}

	for (Enemy* enemy : enemies_) {

		// ShooterEnemy だけ弾を持っている
		ShooterEnemy* shooter = dynamic_cast<ShooterEnemy*>(enemy);
		if (!shooter)
			continue;

		for (EnemyBullet* bullet : shooter->GetBullets()) {

			if (HitSphere(bullet->GetWorldPosition(), bullet->GetRadius(), player_->GetWorldPosition(), player_->GetRadius())) {
				player_->OnHitByBeam();
				bullet->Kill(); // 弾を消す
			}
		}
	}


	// ----------------------
	// 弾 vs 敵 当たり判定
	// ----------------------
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
				enemy->Damage(1); // ダメージ量は好きに
			}
		}
	}

	// ----------------------
	// ミサイル（HomingArcBullet）vs 敵
	// ----------------------
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
				enemy->Damage(3); // ミサイルは強め、とか
			}
		}
	}


	// --- リスポーン待ち ---
	if ((int)enemies_.size() < maxEnemies_) {
		respawnTimer_ -= deltaTime;

		if (respawnTimer_ <= 0.0f) {
			SpawnEnemy();
			respawnTimer_ = 0.0f; // 念のため

			player_->SetEnemies(&enemies_); // ★追加：新しい敵リストを渡し直す
		}
	}

	UpdateWaves();
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
	enemies_.push_back(enemy);
}

void GameScene::SpawnShooterEnemy(const Vector3& pos) {
	Enemy* enemy = new ShooterEnemy();
	enemy->Initialize(enemy_model_, &camera_, pos);
	enemies_.push_back(enemy);
}


void GameScene::SpawnEnemy() {
	float x = (float(rand()) / RAND_MAX) * 12.0f - 6.0f; // -6〜+6
	Vector3 pos = {x, 0.0f, 10.0f};

	Enemy* enemy_Tackler = new TacklerEnemy();
	enemy_Tackler->Initialize(enemy_model_, &camera_, pos);
	enemies_.push_back(enemy_Tackler);
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

void GameScene::SpawnWave(const WaveSpawn& w) {
	for (int i = 0; i < w.tackler; ++i) {
		SpawnTacklerEnemy(RandomSpawnPos());
	}
	for (int i = 0; i < w.shooter; ++i) {
		// 例：SpawnShooterEnemy を用意するか、ここで new して追加
		SpawnShooterEnemy(RandomSpawnPos());
	}
	// turret は後で
}

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
