#pragma once
#include "HomingArcBullet.h"
#include "KamataEngine.h"
#include "MatrixMath.h"
#include "PlayerBullet.h"
#include <list>
#include "Enemy.h"
#include "LockOnManager.h"

using namespace KamataEngine;

class Player {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position, uint32_t lockonTexture);
	void Update();
	void Draw3D();
	void Draw2D();
	void Attack();
	~Player();
	
	void SetEnemies(const std::vector<Enemy*>& enemies);

	void OnHit();

	Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	void OnHitByBeam();

	float GetRadius() const { return radius_; }

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {0, 0, 0};
	std::list<PlayerBullet*> bullets_;
	std::list<HomingArcBullet*> arcBullets_;
	Input* input_ = nullptr;
	std::vector<Enemy*> enemies_;
	std::vector<Enemy*> lockedOnEnemies_;
	std::vector<Enemy*> lockedEnemies_; // ロックオンした敵
	LockOnManager lolckOn_;
	int invincibleTimer_ = 0; // 無敵残りフレーム
	int hitPoint_ = 3;        // 既にあるなら不要

	float radius_ = 0.5f;
};
