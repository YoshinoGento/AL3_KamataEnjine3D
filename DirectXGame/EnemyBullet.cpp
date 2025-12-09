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

bool EnemyBullet::IsHitByPlayerBullet(const Vector3& bulletPosition) {
	// AABB の min / max を計算
	const float kBulletRadius = 0.5f; // プレイヤー当たり判定の半径
	Vector3 minPosition = GetWorldPosition() - kBulletRadius;
	Vector3 maxPosition = GetWorldPosition() + kBulletRadius;

	// 点（弾）の位置が AABB 内にあるか判定
	bool isInside = (bulletPosition.x >= minPosition.x && bulletPosition.x <= maxPosition.x) && (bulletPosition.y >= minPosition.y && bulletPosition.y <= maxPosition.y) &&
	                (bulletPosition.z >= minPosition.z && bulletPosition.z <= maxPosition.z);

	if (isInside) {
		// ヒットしたのでダメージ
		isDead_ = true;

		// 1発で複数部位に当たらないように、ここで終了
		return true;
	}

	return false;
}
