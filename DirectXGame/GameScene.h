#pragma once
#include "GameManager.h"
#include "IScene.h"

#include "KamataEngine.h"
#include <vector>

#include "BarrgeTurretEnemy.h"
#include "Enemy.h"
#include "PauseMenu.h"
#include "Player.h"
#include "ShooterEnemy.h"
#include "SkyDome.h"
#include "SpawnDirector.h"
#include "TacklerEnemy.h"
#include "WaveManager.h"
#include "TutorialUI.h"

using namespace KamataEngine;

class GameScene : public IScene {
public:
	// IScene
	void Initialize(GameManager* manager) override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

	// WaveManager から呼ばれるスポーン関数
	void SpawnTacklerEnemy(const Vector3& pos);
	void SpawnShooterEnemy(const Vector3& pos);
	void SpawnTurretEnemy(const Vector3& pos);

	static bool HitSphere(const Vector3& aPos, float aR, const Vector3& bPos, float bR);

private:
	// ===== シーン管理 =====
	GameManager* manager_ = nullptr;

	// ===== ポーズ =====
	PauseMenu pause_;
	bool isPaused_ = false;

	// ===== ゲーム要素 =====
	Player* player_ = nullptr;
	std::vector<Enemy*> enemies_;

	Model* playerModel_ = nullptr;
	// ★敵モデルを種類ごとに分ける
	Model* tacklerModel_ = nullptr;
	Model* shooterModel_ = nullptr;
	Model* turretModel_ = nullptr;

	// ★（任意）弾モデルも分ける
	Model* enemyBulletModel_ = nullptr;
	Model* playerBulletModel_ = nullptr;
	Model* playerMissile_ = nullptr;

	Camera camera_;
	DebugCamera* debugCamera_ = nullptr;

	// ===== Wave管理 =====
	WaveManager waveManager_;
	int currentWaveIndex_ = 0;

	// ★追加：チュートリアルUIの変数
	TutorialUI tutorialUI_;

	enum class WavePhase { Fighting, Waiting, Clear };
	WavePhase wavePhase_ = WavePhase::Fighting;
	float waveWaitTimer_ = 0.0f;

	SkyDome sky_;

	SpawnDirector spawner_;

	Vector3 GetSafeSpawnPos();

private:
	void UpdateEnemies();
	void ResolveCollisions();
	void RemoveDeadEnemies();
	void UpdateWaves(float dt);

	void UpdatePause(); // ★追加：ポーズ中処理
	void Cleanup();     // ★追加：Finalize用

	void ApplyEnemySeparation(float dt);

private:
	uint32_t bgmGame_ = 0;
	uint32_t seShot_ = 0;
	uint32_t seHit_ = 0;
	uint32_t sePauseOpen_ = 0;

	uint32_t bgmVoice_ = 0;
};