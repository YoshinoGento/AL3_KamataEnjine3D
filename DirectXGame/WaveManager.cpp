#include "WaveManager.h"
#include "GameScene.h" // Spawn関数を呼ぶため

void WaveManager::Initialize() {
	waves_.clear();

	// ==========================================
	// Stage 1: 基本のチュートリアル
	// ==========================================

	// Wave 1: 「接近戦」の練習
	// 正面、左、右から順に突っ込んでくる。落ち着いて狙えば倒せる。
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Tackler, {0, 0, 10} }, // 正面
		    {1.5f, EnemyType::Tackler, {-4, 0, 10}}, // 左
		    {3.0f, EnemyType::Tackler, {4, 0, 10} }, // 右
		};
		w.clearNeedKill = 3;
		waves_.push_back(w);
	}

	// Wave 2: 「遠距離攻撃」への対処
	// 左右同時に出現する射撃敵。かわしながら撃つ練習。
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Shooter, {-3, 1, 12}},
		    {0.5f, EnemyType::Shooter, {3, 1, 12} },
		};
		w.clearNeedKill = 2;
		waves_.push_back(w);
	}

	// ==========================================
	// Stage 2: 配置と連携
	// ==========================================

	// Wave 3: V字フォーメーション（編隊飛行）
	// 敵が隊列を組んでくる「ゲームっぽさ」の演出。
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Tackler, {0, 0, 10} }, // 先頭
		    {0.5f, EnemyType::Tackler, {-2, 0, 11}}, // 左翼
		    {0.5f, EnemyType::Tackler, {2, 0, 11} }, // 右翼
		    {1.0f, EnemyType::Tackler, {-4, 0, 12}}, // 左外
		    {1.0f, EnemyType::Tackler, {4, 0, 12} }, // 右外
		};
		w.clearNeedKill = 5;
		waves_.push_back(w);
	}

	// Wave 4: 盾と槍（連携）
	// 奥に動かない砲台(Barrage)がいて、手前から突進(Tackler)が邪魔をする。
	// 突進を処理しつつ、奥を狙う視線移動が必要。
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Barrage, {0, 1, 15} }, // 奥に鎮座
		    {1.0f, EnemyType::Tackler, {-3, 0, 10}}, // 邪魔役1
		    {2.0f, EnemyType::Tackler, {3, 0, 10} }, // 邪魔役2
		};
		w.clearNeedKill = 3;
		waves_.push_back(w);
	}

	// ==========================================
	// Stage 3: 激化する戦い
	// ==========================================

	// Wave 5: 時間差クロスファイア
	// 左から出た直後、右からも出る。画面全体を見る必要がある。
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Shooter, {-5, 1, 10}},
		    {0.5f, EnemyType::Shooter, {-3, 0, 10}},
		    {2.0f, EnemyType::Shooter, {5, 1, 10} }, // タイミングをずらす
		    {2.5f, EnemyType::Shooter, {3, 0, 10} },
		};
		w.clearNeedKill = 4;
		waves_.push_back(w);
	}

	// Wave 6: 波状攻撃（ラッシュ）
	// 短い間隔で連続して敵が来る。リロードや回避の隙がない緊張感。
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Tackler, {0, 0, 10} },
            {0.8f, EnemyType::Tackler, {-2, 1, 10}},
            {1.6f, EnemyType::Tackler, {2, 1, 10} },
            {2.4f, EnemyType::Shooter, {0, 0, 12} }, // 最後に射撃が混ざる
		};
		w.clearNeedKill = 4;
		waves_.push_back(w);
	}

	// ==========================================
	// Stage 4: クライマックス
	// ==========================================

	// Wave 7: 包囲網
	// 上下左右、広範囲に敵を展開させる。
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Barrage, {-4, 1, 12}}, // 左上
		    {0.0f, EnemyType::Barrage, {4, 1, 12} }, // 右上
		    {1.5f, EnemyType::Tackler, {0, -1, 8} }, // 中央下（不意打ち）
		};
		w.clearNeedKill = 3;
		waves_.push_back(w);
	}

	// Wave 8: 混成部隊（総力戦）
	// 今までの敵がランダムっぽく入り乱れる。
	{
		WaveData w;
		w.events = {
		    {0.0f, EnemyType::Shooter, {-3, 1, 12}},
            {0.5f, EnemyType::Tackler, {0, 0, 10} },
            {1.0f, EnemyType::Shooter, {3, 1, 12} },
            {2.0f, EnemyType::Barrage, {0, 0, 15} }, // 奥から弾幕
		    {2.5f, EnemyType::Tackler, {-4, 0, 8} },
            {2.5f, EnemyType::Tackler, {4, 0, 8}  },
		};
		w.clearNeedKill = 6;
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
