#include "Enemy.h"
#include "GameScene.h"
#include "MatrixMath.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <numbers>

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
}

void Enemy::Update(const Vector3 playerPos) {
	Vector3 toPlayer = playerPos - worldTransform_.translation_;
	toPlayer = Normalized(toPlayer);

	worldTransform_.rotation_.y = std::atan2(toPlayer.x, toPlayer.z);

	attackTimer_++;
	if (attackTimer_ >= 60) {
		Attack(playerPos);
		attackTimer_ = 0;
	}

	for (EnemyBullet* bullet : bullets_) {
		bullet->Update();
	}

	bullets_.erase(
	    std::remove_if(
	        bullets_.begin(), bullets_.end(),
	        [](EnemyBullet* bullet) {
		        if (bullet->IsDead()) {
			        delete bullet;
			        return true;
		        }
		        return false;
	        }),
	    bullets_.end());

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void Enemy::Attack(const Vector3& playerPos) {
	const float kBulletSpeed = 0.3f;

	Vector3 toPlayer = playerPos - worldTransform_.translation_;
	toPlayer = Normalized(toPlayer);
	Vector3 velocity = toPlayer * kBulletSpeed;

	EnemyBullet* newBullet = new EnemyBullet();
	newBullet->Initialize(model_, worldTransform_.translation_, velocity);
	bullets_.push_back(newBullet);
}

void Enemy::Draw() {
	model_->Draw(worldTransform_, *camera_);
	for (EnemyBullet* bullet : bullets_) {
		bullet->Draw(*camera_);
	}
}

Enemy::~Enemy() {
	for (EnemyBullet* bullet : bullets_) {
		delete bullet;
	}
}

Vector3 Enemy::GetWorldPosition() const { return worldTransform_.translation_; }
