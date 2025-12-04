#pragma once
#include "KamataEngine.h"
#include <list>

using namespace KamataEngine;

class MissilePartocle {
public:
	void Initialize(const Vector3& position);
	void Update();
	void Draw(const Camera& camera);
	bool IsDead() const { return isDead_; }

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	float lifetime_ = 30.0f;
	float time_ = 0.0f;
	bool isDead_ = false;
	float alpha_ = 1.0f;
	Vector3 initialScale_ = {1.0f, 1.0f, 1.0f}; // 初期スケール
};
