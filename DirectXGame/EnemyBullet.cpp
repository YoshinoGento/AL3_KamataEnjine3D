#include "EnemyBullet.h"
#include <cassert>

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	assert(model);

	model_ = model;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	velocity_ = velocity;

	
    // ★追加：生成直後に行列を作って転送しておく
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void EnemyBullet::Update() {
	worldTransform_.translation_ += velocity_;

	// ★進行方向に向ける（ミサイルっぽくする）
	if (Length(velocity_) > 1e-6f) {
		worldTransform_.rotation_ = LookRotation(Normalized(velocity_));
	}

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	if (--dethTimer_ <= 0) {
		isDead_ = true;
	}
}

void EnemyBullet::Draw(const Camera& camera) {
	if (!model_) {
		return;
	}
	model_->Draw(worldTransform_, camera); // ★テクスチャ指定しない
}
