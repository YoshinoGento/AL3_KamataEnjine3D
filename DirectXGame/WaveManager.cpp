#include "WaveManager.h"
#include "GameScene.h" // Spawn関数を呼ぶため

void WaveManager::Initialize() {
	waves_.clear();

	// -------------------------
	// Wave1：突進だけ（導入）
	// -------------------------
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Tackler, {-4, 0, 10}},
		    {0.5f, EnemyType::Tackler, {0, 0, 10} },
		    {1.0f, EnemyType::Tackler, {4, 0, 10} },
		};
		w.clearNeedKill = 3;
		waves_.push_back(w);
	}

	// -------------------------
	// Wave2：射撃を混ぜる
	// -------------------------
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Shooter, {-4, 1, 10}},
		    {0.8f, EnemyType::Tackler, {0, 0, 10} },
		    {1.6f, EnemyType::Shooter, {4, 1, 10} },
		};
		w.clearNeedKill = 3;
		waves_.push_back(w);
	}

	// -------------------------
	// Wave3：3種類目も出す（例：弾幕）
	// ※まだ3種類目が無ければ、ここは後でOK
	// -------------------------
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Barrage, {0, 1, 10} },
		    {0.7f, EnemyType::Tackler, {-4, 0, 10}},
		    {1.4f, EnemyType::Shooter, {4, 1, 10} },
		};
		w.clearNeedKill = 3;
		waves_.push_back(w);
	}
}

void WaveManager::StartWave(int waveIndex) {
	waveIndex_ = waveIndex;
	waveTimer_ = 0.0f;
	nextEventIndex_ = 0;
	killedInThisWave_ = 0;
	finished_ = false;
}

void WaveManager::Update(float deltaTime, GameScene& scene, int enemiesAliveCount) {
	if (finished_)
		return;
	if (waveIndex_ < 0 || waveIndex_ >= (int)waves_.size())
		return;

	waveTimer_ += deltaTime;

	WaveData& w = waves_[waveIndex_];

	// ① 時間になったら台本通りにスポーン
	while (nextEventIndex_ < (int)w.events.size()) {
		const SpawnEvent& e = w.events[nextEventIndex_];

		if (waveTimer_ < e.timeFromWaveStart) {
			break; // まだ時間じゃない
		}

		Spawn(scene, e.type, e.position);
		nextEventIndex_++;
	}

	// ② クリア判定（簡易版）
	// 台本のスポーンが全部終わっていて、敵が0体ならWave終了
	const bool allSpawned = (nextEventIndex_ >= (int)w.events.size());
	if (allSpawned && enemiesAliveCount == 0) {
		finished_ = true;
	}
}

void WaveManager::Spawn(GameScene& scene, EnemyType type, const Vector3& pos) {
	switch (type) {
	case EnemyType::Tackler:
		scene.SpawnTacklerEnemy(pos);
		break;
	case EnemyType::Shooter:
		scene.SpawnShooterEnemy(pos);
		break;
	case EnemyType::Barrage:
		// 3種類目（例）
		scene.SpawnTurretEnemy(pos);
		break;
	}
}
