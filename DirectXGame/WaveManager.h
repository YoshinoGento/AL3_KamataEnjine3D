#pragma once
#include "KamataEngine.h"
#include <vector>

using namespace KamataEngine;

class GameScene; // 前方宣言（GameSceneのSpawn関数を呼ぶ）

class WaveManager {
public:
	// 敵の種類（3種類想定）
	enum class EnemyType {
		Tackler, // 突進
		Shooter, // 6発→移動
		Barrage  // 3種類目（例：弾幕砲台）※まだ無ければ後で追加でOK
	};

	// いつ・何を・どこに出すか（台本の1行）
	struct SpawnEvent {
		float timeFromWaveStart; // Wave開始から何秒後に出すか
		EnemyType type;
		Vector3 position;
	};

	// Waveの台本
	struct WaveData {
		std::vector<SpawnEvent> events;
		int clearNeedKill = 0; // このWaveで倒すべき敵数（簡易）
	};

public:
	void Initialize();
	void StartWave(int waveIndex);

	// enemiesAliveCount は「今敵が何体いるか」を GameScene 側から渡す
	void Update(float deltaTime, GameScene& scene, int enemiesAliveCount);

	bool IsFinished() const { return finished_; }
	int GetWaveIndex() const { return waveIndex_; }

	int GetWaveCount() const { return (int)waves_.size(); }


private:
	void Spawn(GameScene& scene, EnemyType type, const Vector3& pos);

private:
	std::vector<WaveData> waves_;

	int waveIndex_ = 0;
	float waveTimer_ = 0.0f;
	int nextEventIndex_ = 0;

	int killedInThisWave_ = 0; // 「倒した数」扱い（簡易）
	bool finished_ = false;
};
