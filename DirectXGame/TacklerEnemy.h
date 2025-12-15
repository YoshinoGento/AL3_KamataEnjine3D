#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
using namespace KamataEngine;

class TacklerEnemy : public Enemy {
	void Initialize(Model* model, Camera* camera, const Vector3& position) override;
	void Update(const Vector3& playerPos) override;
	void Draw3D() override;

private:
	Vector3 moveDir_{0, 0, 1};  // 今向いている方向
	float speed_ = 0.15f;       // タックル速度
	float homingPower_ = 0.05f; // ← タックル用ホーミング（弾より強め）
};
