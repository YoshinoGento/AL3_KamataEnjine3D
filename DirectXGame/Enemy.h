#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include <list>

using namespace KamataEngine;

class Enemy {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update(Vector3 playerPos);
	void Attack(const Vector3& playerPos);
	void Draw();
	~Enemy();

	Vector3 GetWorldPosition() const;

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Camera* camera_ = nullptr;
	std::vector<EnemyBullet*> bullets_;
	int attackTimer_ = 0;
};
