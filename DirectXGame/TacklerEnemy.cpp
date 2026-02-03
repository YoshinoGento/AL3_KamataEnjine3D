#include "TacklerEnemy.h"
#include "MatrixMath.h"

void TacklerEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	Enemy::Initialize(model, camera, position);

	// 最初は前向き
	moveDir_ = {0, 0, 1};

	spawnPos_ = position;

	lockedOn_ = false;
	passedPlayer_ = false;

	homingTimer_ = 0.0f;
}

void TacklerEnemy::Update(const Vector3& playerPos) {

	// ① 最初の1回だけ、突進方向をプレイヤーへ固定
	if (!lockedOn_) {
		moveDir_ = Normalized(playerPos - basePos_); // ★修正：basePos_基準
		lockedOn_ = true;
	}
	// ② 追尾を少しだけ残したいなら（超弱）
	else if (!passedPlayer_) {
		const float dt = 1.0f / 60.0f;
		homingTimer_ += dt;

		if (homingTimer_ <= homingTime_) {
			Vector3 targetDir = Normalized(playerPos - basePos_); // ★修正
			moveDir_ = Normalized(moveDir_ + (targetDir - moveDir_) * homingPower_);
		}
	}

	// ③ 移動（突進）
	// ★修正：見た目ではなく「本来の座標」を動かす
	basePos_ += moveDir_ * speed_;

	// ==================================================
	// ★親クラス更新（ここで basePos_ + 演出 = worldTransform_ が計算される）
	// ==================================================
	Enemy::Update(playerPos);

	// ④ 向き（進行方向へ）
	worldTransform_.rotation_ = LookRotation(moveDir_);

	// ★モデルが逆向きなら 180°回して補正（前後が逆）
	worldTransform_.rotation_.y += 3.14159265f; // = π

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	// =========================
	// ⑤ 通り過ぎ判定
	// =========================
	Vector3 toPlayer = playerPos - basePos_; // ★修正
	float forward = Dot(moveDir_, toPlayer);

	// forward < 0 になったら「プレイヤーが後ろに回った」＝通り過ぎ
	if (!passedPlayer_ && forward < 0.0f) {
		passedPlayer_ = true;
		spawnPos_ = basePos_; // ★修正
	}

	// ⑥ 消滅判定
	float dist = Length(basePos_ - spawnPos_); // ★修正
	float limit = passedPlayer_ ? despawnAfterPass_ : maxTravelDistance_;

	if (dist > limit) {
		isDead_ = true;
	}
}

void TacklerEnemy::Draw3D() { Enemy::Draw3D(); }