#include "TacklerEnemy.h"
#include "MatrixMath.h"

void TacklerEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) {

	Enemy::Initialize(model, camera, position);

	// 最初は前向き
	moveDir_ = {0, 0, 1};

	spawnPos_ = position; // ★追加：消える判定用

	lockedOn_ = false;
	passedPlayer_ = false;

	homingTimer_ = 0.0f;
}

void TacklerEnemy::Update(const Vector3& playerPos) {

	// ① 最初の1回だけ、突進方向をプレイヤーへ固定
	if (!lockedOn_) {
		moveDir_ = Normalized(playerPos - worldTransform_.translation_);
		lockedOn_ = true;
	}
	// ② 追尾を少しだけ残したいなら（超弱）
	else if (!passedPlayer_) {
		const float dt = 1.0f / 60.0f;
		homingTimer_ += dt;

		if (homingTimer_ <= homingTime_) {
			Vector3 targetDir = Normalized(playerPos - worldTransform_.translation_);
			moveDir_ = Normalized(moveDir_ + (targetDir - moveDir_) * homingPower_);
		}
	}

	// ③ 移動（突進）
	worldTransform_.translation_ += moveDir_ * speed_;

	// ④ 向き
	worldTransform_.rotation_ = LookRotation(moveDir_);

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	// =========================
	// ⑤ 通り過ぎ判定
	// =========================
	Vector3 toPlayer = playerPos - worldTransform_.translation_;
	float forward = Dot(moveDir_, toPlayer);

	// forward < 0 になったら「プレイヤーが後ろに回った」＝通り過ぎ
	if (!passedPlayer_ && forward < 0.0f) {
		passedPlayer_ = true;
		spawnPos_ = worldTransform_.translation_; // ★ここを基準にして「あと少し進んだら消す」
	}

	// 通り過ぎた後、少し進んだら消える
	if (passedPlayer_) {
		float after = Length(worldTransform_.translation_ - spawnPos_);
		if (after > despawnAfterPass_) {
			isDead_ = true;
		}
	}

	// ⑥ 保険：通り過ぎる前だけ行き過ぎチェック（spawnPos_ を上書きするため）
	if (!passedPlayer_) {
		float traveled = Length(worldTransform_.translation_ - spawnPos_);
		if (traveled > maxTravelDistance_) {
			isDead_ = true;
		}
	}
}




void TacklerEnemy::Draw3D() {
	if (!model_ || !camera_) {
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}
