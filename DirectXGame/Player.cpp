#include "Player.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

void Player::Initialize(Model* playerModel, Model* playerBulletModel, Camera* camera, const Vector3& position) {
	assert(playerModel);
	assert(playerBulletModel);
	model_ = playerModel;
	player_bullet_model_ = playerBulletModel;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	input_ = Input::GetInstance();
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

void Player::Draw() {
	model_->Draw(worldTransform_, *camera_);

	for (PlayerBullet* bullet : bullets_) {
		bullet->Draw(*camera_);
	}
	for (HomingArcBullet* bullet : arcBullets_) {
		bullet->Draw(*camera_);
	}
}

void Player::Attack() {
	if (input_->TriggerKey(DIK_SPACE)) {
		const float kBulletSpeed = 1.0f;
		Vector3 velocity(0, 0, kBulletSpeed);
		velocity = TransformNormal(velocity, worldTransform_.matWorld_);

		PlayerBullet* newBullet = new PlayerBullet();
		newBullet->Initialize(player_bullet_model_, worldTransform_.translation_, velocity);
		bullets_.push_back(newBullet);
	}

	// ロックオン処理（右クリックなど）
	if (input_->IsTriggerMouse(1)) { // 右クリックでロックオン
		lockedEnemy_ = enemy_;
	}

	// 発射（左クリック）
	if (input_->IsTriggerMouse(0)) {
		if (lockedEnemy_) {
			Vector3 enemyPos = lockedEnemy_->GetWorldPosition();

			HomingArcBullet* arcBullet = new HomingArcBullet();
			arcBullet->Initialize(player_bullet_model_, worldTransform_.translation_, enemyPos);
			arcBullets_.push_back(arcBullet);
		}
	}

	// キーでロック解除（例：Rキー）
	if (input_->TriggerKey(DIK_R)) {
		lockedEnemy_ = nullptr;
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

void Player::SetEnemy(Enemy* enemy) { enemy_ = enemy; }
