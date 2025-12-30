#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include "TacklerEnemy.h"
#include "ShooterEnemy.h"
#include "BarrgeTurretEnemy.h"
#include "WaveManager.h"

using namespace KamataEngine;

enum class WaveState {
	Fighting,
	WaitingNext,
	Clear
};

struct WaveSpawn {
	int tackler = 0;
	int shooter = 0;
	int turret = 0; // 後で作る固定砲台（未実装なら0でOK）
};

class GameScene {
public:
	void Initialize();
	void Update();
	void Draw();
	void Delete();
	void SpawnTacklerEnemy(const Vector3& pos);
	void SpawnShooterEnemy(const Vector3& pos);
	void SpawnEnemy();
	void SpawnTurretEnemy(const Vector3& pos);



	static bool HitSphere(const Vector3& aPos, float aR, const Vector3& bPos, float bR);

	static int CountTacklers(const std::vector<Enemy*>& enemies);

	static int CountShooters(const std::vector<Enemy*>& enemies);


	WaveState waveState_ = WaveState::Fighting;
	int waveIndex_ = 0;
	std::vector<WaveSpawn> waves_;

	void InitWaves();
	void UpdateWaves();
	bool AreAllEnemiesDead() const;
	void SpawnWave(const WaveSpawn& w);
	Vector3 RandomSpawnPos() const;


private:
	Player* player_ = nullptr;
	Enemy* enemy_ = nullptr;
	std ::vector<Enemy*> enemies_;
	Model* player_model_ = nullptr;
	Model* enemy_model_ = nullptr;
	Camera camera_;
	Camera PlayerCamera_;
	Camera EnemyCamera_;
	uint32_t textureHandle_ = 0u;
	DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;
	int maxEnemies_ = 5;         // 常にこの数を維持
	float respawnZ_ = 10.0f;     // リスポーンZ
	float respawnXRange_ = 6.0f; // -6〜+6 に出す
	float respawnTimer_ = 0.0f;
	const float respawnInterval_ = 1.0f; // 1秒

	
	// 目標数
	static constexpr int kTargetTacklers = 3;
	static constexpr int kTargetShooters = 3;

	WaveManager waveManager_;
	int currentWave_ = 0;

	enum class WavePhase { Fighting, Waiting, Clear };
	WavePhase wavePhase_ = WavePhase::Fighting;
	float waveWaitTimer_ = 0.0f;
};
