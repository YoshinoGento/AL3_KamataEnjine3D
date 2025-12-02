#pragma once
#include "Enemy.h"
#include "HomingArcBullet.h"
#include "KamataEngine.h"
#include "MatrixMath.h"
#include "MissilePartocle.h"
#include "PlayerBullet.h"
#include <list>

using namespace KamataEngine;

class Player {
public:
	void Initialize(Model* playerModel, Model* playerBulletModel, Camera* camera, const Vector3& position);
	void Update();
	void Draw();
	void Attack();
	~Player();
	Vector3 GetWorldPosition() const;
	void SetEnemy(Enemy* enemy);

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Model* player_bullet_model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {0, 0, 0};
	std::list<PlayerBullet*> bullets_;
	std::list<HomingArcBullet*> arcBullets_;
	Input* input_ = nullptr;
	Enemy* enemy_ = nullptr;
	std::vector<Enemy*> lockedOnEnemies_;
	bool isFiringFanMissiles_ = false; // ファンネル弾発射中フラグ
	int fireInterval_ = 5;             // フレーム間隔
	int fireTimer_ = 0;                // タイマー
	int fireCount_ = 0;                // 発射済み弾数（最大6）
	Enemy* lockedEnemy_ = nullptr;     // ロックオンした敵
	std::list<MissilePartocle*> missileParticles_;
};
