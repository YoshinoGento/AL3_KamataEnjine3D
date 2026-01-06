#include "Enemy.h"
#include "MatrixMath.h"
#include <cassert>
#include <cmath>

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

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
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

	// ★上下が逆ならこれを入れる（まずここが本命）
	dir.y *= -1.0f;

	worldTransform_.rotation_ = LookRotation(dir);

	// ★前後が逆なら 180度回す（必要な場合だけ）
	worldTransform_.rotation_.y += 3.14159265f;

	// ★ここでは Transfer しない（Update側でまとめてやる）
}


bool Enemy::CheckCollision(const Vector3& playerPos, float playerRadius) {
	Vector3 diff = playerPos - worldTransform_.translation_;
	float dist = Length(diff);
	return dist < (radius_ + playerRadius);
}
