#include "EnemyBullet.h"
#include <cassert>

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	assert(model);

	model_ = model;
	textureHandle_ = TextureManager::Load("SP.png");
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	velocity_ = velocity;
}

void EnemyBullet::Update() {
	worldTransform_.translation_ += velocity_;
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	if (--dethTimer_ <= 0) {
		isDead_ = true;
	}
}

void EnemyBullet::Draw(const Camera& camera) { model_->Draw(worldTransform_, camera, textureHandle_); }
