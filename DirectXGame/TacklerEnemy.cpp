#include "TacklerEnemy.h"
#include "MatrixMath.h"

void TacklerEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) {

	Enemy::Initialize(model, camera, position);

	// 最初は前向き
	moveDir_ = {0, 0, 1};
}

void TacklerEnemy::Update(const Vector3& playerPos) {

	// ① プレイヤーへの方向
	Vector3 targetDir = Normalized(playerPos - worldTransform_.translation_);

	// ② 今の進行方向を少しだけ曲げる（弱ホーミング）
	moveDir_ = Normalized(moveDir_ + (targetDir - moveDir_) * homingPower_);

	// ③ 移動（タックル）
	worldTransform_.translation_ += moveDir_ * speed_;

	// ④ 向きを進行方向に合わせる（見た目用）
	worldTransform_.rotation_ = LookRotation(moveDir_);

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}


void TacklerEnemy::Draw3D() {
	if (!model_ || !camera_) {
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}
