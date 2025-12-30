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

	// ★追加：ここで最初の行列を確定＆GPUへ
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
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

void Enemy::Damage(int amount) {
	hp_ -= amount;
	if (hp_ <= 0) {
		hp_ = 0;
		isDead_ = true;
	}
}

void Enemy::FaceTo(const Vector3& targetPos) {
	Vector3 dir = targetPos - worldTransform_.translation_;
	if (Length(dir) < 1e-6f) {
		dir = {0, 0, 1};
	}
	dir = Normalized(dir);

	worldTransform_.rotation_ = LookRotation(dir);

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}


bool Enemy::CheckCollision(const Vector3& playerPos, float playerRadius) {
	Vector3 diff = playerPos - worldTransform_.translation_;
	float dist = Length(diff);
	return dist < (radius_ + playerRadius);
}
