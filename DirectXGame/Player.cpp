#include "Player.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

void Player::Initialize(Model* model, Camera* camera, const Vector3& position, uint32_t lockonTexture) {
	assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	input_ = Input::GetInstance();
	lolckOn_.Initialize(lockonTexture);
}

void Player::Update() {
	bullets_.remove_if([](PlayerBullet* bullet) {
		if (bullet->IsDead()) {
			delete bullet;
			return true;
		}
		return false;
	});

	Vector3 move = {0.0f, 0.0f, 0.0f};
	const float kCharacterSpeed = 0.01f;
	const float kFriction = 0.9f;
	const float kMaxSpeed = 3.0f;

	if (input_->PushKey(DIK_W)) {
		move.y += kCharacterSpeed;
	}
	if (input_->PushKey(DIK_S)) {
		move.y -= kCharacterSpeed;
	}
	if (input_->PushKey(DIK_D)) {
		move.x += kCharacterSpeed;
	}
	if (input_->PushKey(DIK_A)) {
		move.x -= kCharacterSpeed;
	}

	velocity_ += move;
	if (Length(velocity_) > kMaxSpeed) {
		velocity_ = Normalized(velocity_) * kMaxSpeed;
	}

	velocity_ *= kFriction;
	worldTransform_.translation_ += velocity_;

	const float kMoveLimitX = 6.0f;
	const float kMoveLimitY = 3.0f;

	worldTransform_.translation_.x = std::clamp(worldTransform_.translation_.x, -kMoveLimitX, +kMoveLimitX);
	worldTransform_.translation_.y = std::clamp(worldTransform_.translation_.y, -kMoveLimitY, +kMoveLimitY);

	Attack();

	if (input_->IsTriggerMouse(1)) {
		lolckOn_.TryLockOn(input_->GetMousePosition(), enemies_, *camera_);
	}

	for (PlayerBullet* bullet : bullets_) {
		bullet->Update();
	}

	for (HomingArcBullet* arcBullet : arcBullets_) {
		arcBullet->Update();
	}

	arcBullets_.remove_if([](HomingArcBullet* b) {
		if (b->IsDead()) {
			delete b;
			return true;
		}
		return false;
	});

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void Player::Draw3D() {
	model_->Draw(worldTransform_, *camera_);
	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(*camera_);
	}
	for (HomingArcBullet* bullet : arcBullets_) {
		bullet->Draw(*camera_);
	}
}

void Player::Draw2D() { 
	lolckOn_.DrawMarkers(*camera_); 
}
void Player::Attack() {

	// ---------- 通常弾 ----------
	if (input_->TriggerKey(DIK_SPACE)) {
		const float kBulletSpeed = 1.0f;
		Vector3 velocity(0, 0, kBulletSpeed);
		velocity = TransformNormal(velocity, worldTransform_.matWorld_);

		PlayerBullet* bullet = new PlayerBullet();
		bullet->Initialize(model_, worldTransform_.translation_, velocity);
		bullets_.push_back(bullet);
	}

	// ---------- 右クリック：ロックオン ----------
	if (input_->IsTriggerMouse(1)) {
		lolckOn_.TryLockOn(input_->GetMousePosition(), enemies_, *camera_);
	}

	// ---------- 左クリック：ミサイル発射 ----------
	if (input_->IsTriggerMouse(0)) {

		for (Enemy* enemy : lolckOn_.GetLockedEnemies()) {

			HomingArcBullet* arc = new HomingArcBullet();

			// ★ 軌道用オフセット（ミサイルごとに違う）
			Vector3 offset = {
			    (rand() % 200 - 100) / 10.0f,  // X：左右に散る
			    (rand() % 100) / 10.0f + 5.0f, // Y：上に持ち上げる
			    (rand() % 200 - 100) / 10.0f   // Z：前後に散る
			};

			// ★ Initialize の引数を1個増やす
			arc->Initialize(model_, worldTransform_.translation_, enemy->GetWorldPosition(), offset);

			arcBullets_.push_back(arc);
		}

		lolckOn_.Clear();
	}


	// ---------- R：解除 ----------
	if (input_->TriggerKey(DIK_R)) {
		lolckOn_.Clear();
	}
}



Player::~Player() {
	for (PlayerBullet* bullet : bullets_) {
		delete bullet;
	}
	for (HomingArcBullet* bullet : arcBullets_) {
		delete bullet;
	}
}


void Player::SetEnemies(const std::vector<Enemy*>& enemies) { enemies_ = enemies; }


void Player::OnHit() {
	if (invincibleTimer_ > 0)
		return;

	--hitPoint_;
	if (hitPoint_ < 0)
		hitPoint_ = 0;

	invincibleTimer_ = 60; // 無敵時間
}

void Player::OnHitByBeam() {

	// 無敵時間中なら何もしない
	if (invincibleTimer_ > 0) {
		return;
	}

	// HP 減らす
	hitPoint_--;

	// 無敵時間を付与（1秒）
	invincibleTimer_ = 60;

	if (hitPoint_ <= 0) {
		hitPoint_ = 0;
		// TODO: 死亡処理（今は空でOK）
	}
}
