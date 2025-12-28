#include "EnemyBullet.h"
#include <cassert>

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	assert(model);

	model_ = model;
	textureHandle_ = TextureManager::Load("enemyBullet.png");
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	velocity_ = velocity;

	
    // ★追加：生成直後に行列を作って転送しておく
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void EnemyBullet::Update() {
	worldTransform_.translation_ += velocity_;
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	if (--dethTimer_ <= 0) {
		isDead_ = true;
	}
}

void EnemyBullet::Draw(const Camera& camera) { 
	model_->Draw(worldTransform_, camera, textureHandle_);
}
