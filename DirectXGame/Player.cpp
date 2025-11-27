#include "Player.h"
#include "MatrixMath.h"
#include <cassert>
#include <cmath>
#include <numbers>
#include <algorithm>


void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
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
		newBullet->Initialize(model_, worldTransform_.translation_, velocity);
		bullets_.push_back(newBullet);
	}

	if (input_->IsTriggerMouse(1)) {
		Vector2 mousePos = input_->GetMousePosition();

		// 仮にスクリーン→ワールド変換を自前でやる場合（Zは仮固定）
		Vector3 mouseWorldPos = {
		    mousePos.x / 100.0f - 6.0f, mousePos.y / 100.0f - 4.0f,
		    10.0f // 敵のZ座標に合わせておく
		};

		const float lockOnDistance = 1.5f; // ロック可能な距離（調整可）

		for (Enemy* enemy : enemies_) {
			Vector3 enemyPos = enemy->GetWorldPosition();
			float dist = Length(mouseWorldPos - enemyPos);

			if (dist < lockOnDistance) {
				// まだロックしてなければ追加
				if (std::find(lockedEnemies_.begin(), lockedEnemies_.end(), enemy) == lockedEnemies_.end()) {
					lockedEnemies_.push_back(enemy);
				}
			}
		}
	}

	// 発射（左クリック）
	if (input_->IsTriggerMouse(0)) {
		for (Enemy* enemy : lockedEnemies_) {
			Vector3 enemyPos = enemy->GetWorldPosition();

			HomingArcBullet* arcBullet = new HomingArcBullet();
			arcBullet->Initialize(model_, worldTransform_.translation_, enemyPos);
			arcBullets_.push_back(arcBullet);
		}

		// 発射後はロック解除（必要であれば）
		lockedEnemies_.clear();
	}

	// キーでロック解除（例：Rキー）
	if (input_->TriggerKey(DIK_R)) {
		lockedEnemies_.clear();
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

void Player::SetEnemies(const std::vector<Enemy*>& enemies) {
	enemies_ = enemies; 
}
