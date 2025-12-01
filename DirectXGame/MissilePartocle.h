#pragma once
#include "KamataEngine.h"
using namespace KamataEngine;

class MissileParticle {
public:
	void Initialize(const Vector3& position);
	void Update();
	void Draw(const Camera& camera);
	bool IsDead() const { return life_ <= 0; }

private:
	WorldTransform worldTransform_;
	Vector3 velocity_;
	int life_ = 30;
	Model* model_ = nullptr; // 使いまわしの白球モデルなど
};
