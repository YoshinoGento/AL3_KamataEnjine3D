#pragma once
#include "KamataEngine.h"
#include "MatrixMath.h"

using namespace KamataEngine;

class EnemyBullet {
public:
	void Initialize(Model* model, const Vector3& position, const Vector3& velocity);
	void Update();
	void Draw(const Camera& camera);
	bool IsDead() const { return isDead_; }
	Vector3 GetWorldPosition() const { return worldTransform_.translation_; }
	float GetRadius() const { return radius_; }
	void OnCollision() { isDead_ = true; }
	void Kill() { isDead_ = true; }

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Vector3 velocity_{0.0f, 0.0f, 0.0f};
	static const int32_t kLifeTime = 60 * 5;
	int32_t dethTimer_ = kLifeTime;
	bool isDead_ = false;
	float radius_ = 0.35f;
};
