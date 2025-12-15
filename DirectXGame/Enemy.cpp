#include "Enemy.h"
#include "MatrixMath.h"
#include <cassert>

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
}

void Enemy::Update(const Vector3& playerPos) {
	// 基本は何もしない（派生で上書き）
	(void)playerPos;
}

void Enemy::Draw3D() {
	if (model_) {
		model_->Draw(worldTransform_, *camera_);
	}
}

bool Enemy::CheckCollision(const Vector3& playerPos, float playerRadius) {
	Vector3 diff = playerPos - worldTransform_.translation_;
	float dist = Length(diff);
	return dist < (radius_ + playerRadius);
}
