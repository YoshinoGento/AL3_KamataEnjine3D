#pragma once
#include "KamataEngine.h"
#include <vector>

using namespace KamataEngine;

class Enemy {
public:
	virtual ~Enemy() = default;

	virtual void Initialize(Model* model, Camera* camera, const Vector3& position);
	virtual void Update(const Vector3& playerPos);
	virtual bool CheckCollision(const Vector3& playerPos, float playerRadius);

	Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	virtual void Draw3D();

	bool IsDead() const { return isDead_; }
	
	void Kill() { isDead_ = true; } // ★追加：強制的に死ぬ

	void Damage(int amount);

	float GetRadius() const { return radius_; }

protected:
	// ★追加
	bool isDead_ = false;

	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;

	float radius_ = 1.0f;
	int hp_ = 1;
};