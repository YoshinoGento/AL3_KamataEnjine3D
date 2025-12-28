#include "ShooterEnemy.h"
#include <cstdlib>

static float RandomRange(float a, float b) {
	float t = float(rand()) / float(RAND_MAX);
	return a + (b - a) * t;
}

void ShooterEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	Enemy::Initialize(model, camera, position);

	hp_ = 3;
	radius_ = 1.2f;

	bulletModel_ = model_; // とりあえず敵モデルを弾に使う
	fixedZ_ = position.z;

	state_ = State::Shoot;
	shotsLeft_ = 6;
	shotTimer_ = 0;
}

void ShooterEnemy::Update(const Vector3& playerPos) {

	// ---- 敵弾の更新 ----
	for (auto it = bullets_.begin(); it != bullets_.end();) {
		EnemyBullet* b = *it;
		b->Update();
		if (b->IsDead()) {
			delete b;
			it = bullets_.erase(it);
		} else {
			++it;
		}
	}

	// ---- 状態ごとの行動 ----
	switch (state_) {
	case State::Shoot:
		UpdateShoot(playerPos);
		break;

	case State::Move:
		UpdateMove();
		break;
	}

	// 見た目：プレイヤーの方向を向く
	Vector3 dir = Normalized(playerPos - worldTransform_.translation_);
	worldTransform_.rotation_ = LookRotation(dir);

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void ShooterEnemy::UpdateShoot(const Vector3& playerPos) {
	if (shotTimer_ > 0) {
		--shotTimer_;
		return;
	}

	Shoot(playerPos);
	--shotsLeft_;
	shotTimer_ = shotInterval_;

	if (shotsLeft_ <= 0) {
		BeginMove();
		state_ = State::Move;
	}
}

void ShooterEnemy::Shoot(const Vector3& playerPos) {
	Vector3 dir = Normalized(playerPos - worldTransform_.translation_);
	Vector3 vel = dir * bulletSpeed_;

	EnemyBullet* b = new EnemyBullet();
	b->Initialize(bulletModel_, worldTransform_.translation_, vel);
	bullets_.push_back(b);
}

void ShooterEnemy::BeginMove() {
	moveStart_ = worldTransform_.translation_;
	moveTarget_ = {RandomRange(minX_, maxX_), RandomRange(minY_, maxY_), fixedZ_};
	moveT_ = 0.0f;
}

void ShooterEnemy::UpdateMove() {
	moveT_ += moveSpeed_;
	if (moveT_ >= 1.0f)
		moveT_ = 1.0f;

	worldTransform_.translation_ = Lerp(moveStart_, moveTarget_, moveT_);

	if (moveT_ >= 1.0f) {
		shotsLeft_ = 6;
		shotTimer_ = 30;
		state_ = State::Shoot;
	}
}

void ShooterEnemy::Draw3D() {
	if (model_ && camera_) {
		model_->Draw(worldTransform_, *camera_);
	}
	for (EnemyBullet* b : bullets_) {
		b->Draw(*camera_);
	}
}
