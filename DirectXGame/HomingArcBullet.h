#pragma once
#include "KamataEngine.h"
#include "MatrixMath.h"

using namespace KamataEngine;

class HomingArcBullet {
public:
	void Initialize(Model* model, const Vector3& start, const Vector3& target);
	void Update();
	void Draw(const Camera& camera);
	bool IsDead() const { return isDead_; }

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Vector3 start_;
	Vector3 target_;
	Vector3 velocity_;        
	static const int32_t kLifeTime = 60 * 5; 
	float time_ = 0.0f;       // 時間経過
	float lifeTime_ = 120.0f; // 寿命
	bool isDead_ = false;
	
};
