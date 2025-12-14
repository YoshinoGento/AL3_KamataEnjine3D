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
			arc->Initialize(model_, worldTransform_.translation_, enemy->GetWorldPosition());
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

Vector3 Player::GetWorldPosition() const { return worldTransform_.translation_; }

void Player::SetEnemies(const std::vector<Enemy*>& enemies) { enemies_ = enemies; }
