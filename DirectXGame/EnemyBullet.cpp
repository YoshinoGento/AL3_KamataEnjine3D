#include "EnemyBullet.h"
#include <cassert>
#include "MatrixMath.h"
#include "Player.h"

void EnemyBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {
	assert(model);

	model_ = model;
	textureHandle_ = TextureManager::Load("enemyBullet.png");
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	velocity_ = velocity;
}

void EnemyBullet::Update() {
	// 敵弾から自キャラへのベクトルを計算
	Vector3 toPlayer = player_->GetWorldPosition() - GetWorldPosition();

	// ベクトルを正規化する
	Normalized(toPlayer);
	Normalized(velocity_);
	// 球面線形補間
	velocity_ = Slerp(velocity_, toPlayer, t) * kBulletSpeed;

	// Y軸回り角度(θy)
	worldTransform_.rotation_.y = atan2(velocity_.x, velocity_.z);
	// 横軸方向の長さを求める
	float velocityXZ = Length(Vector3{velocity_.x, 0.0f, velocity_.z});
	// X回り角度(θx)a
	worldTransform_.rotation_.x = atan2(-velocity_.y, velocityXZ);

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
