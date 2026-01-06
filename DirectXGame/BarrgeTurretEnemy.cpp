#include "BarrgeTurretEnemy.h"
#include "MatrixMath.h"
#include <cmath>

// 円周率（C++20なら std::numbers::pi でもOK）
static const float kPi = 3.1415926535f;

void BarrageTurretEnemy::Initialize(Model* model, Camera* camera, const Vector3& position) {
	Enemy::Initialize(model, camera, position);

	hp_ = 5;
	radius_ = 1.5f;

	shotTimer_ = 30;
	pattern_ = Pattern::Circle;
	fixedZ_ = position.z;

	// ★Setされてなければ従来通り
	if (!bulletModel_) {
		bulletModel_ = model_;
	}
}

void BarrageTurretEnemy::Update(const Vector3& playerPos) {
	UpdateBullets();

	if (enableMove_) {
		// 1) 移動
		worldTransform_.translation_ += velocity_;
		worldTransform_.translation_.z = fixedZ_;

		// 2) 壁反射（X）
		if (worldTransform_.translation_.x < minX_) {
			worldTransform_.translation_.x = minX_;
			velocity_.x *= -1.0f;
		}
		if (worldTransform_.translation_.x > maxX_) {
			worldTransform_.translation_.x = maxX_;
			velocity_.x *= -1.0f;
		}

		// 3) 壁反射（Y）
		if (worldTransform_.translation_.y < minY_) {
			worldTransform_.translation_.y = minY_;
			velocity_.y *= -1.0f;
		}
		if (worldTransform_.translation_.y > maxY_) {
			worldTransform_.translation_.y = maxY_;
			velocity_.y *= -1.0f;
		}
	}

	if (shotTimer_ > 0) {
		--shotTimer_;
	} else {
		//if (pattern_ == Pattern::Circle) {
			//ShootBarrageCircle(playerPos);
		//} else {
			//ShootBarrageFan();
		//}
		ShootBarrageWall(playerPos);
		shotTimer_ = shotInterval_;
	}

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void BarrageTurretEnemy::UpdateBullets() {
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
}

void BarrageTurretEnemy::ShootOnce(const Vector3& dir) {
	Vector3 d = dir;
	if (Length(d) < 1e-6f) {
		d = {1.0f, 0.0f, 0.0f};
	}
	d = Normalized(d);

	EnemyBullet* b = new EnemyBullet();
	b->Initialize(bulletModel_, worldTransform_.translation_, d * bulletSpeed_);
	bullets_.push_back(b);
}

void BarrageTurretEnemy::ShootBarrageWall(const Vector3& playerPos) {

	Vector3 toPlayer = playerPos - worldTransform_.translation_;
	Vector3 baseDir = (Length(toPlayer) < 1e-6f) ? Vector3{0.0f, 0.0f, -1.0f} : Normalized(toPlayer);

	Vector3 tmpUp = {0.0f, 1.0f, 0.0f};
	if (std::fabs(Dot(baseDir, tmpUp)) > 0.9f)
		tmpUp = {1.0f, 0.0f, 0.0f};

	Vector3 right = Normalized(Cross(baseDir, tmpUp));

	// 壁の幅（大きいほど横に広がる）
	const float width = 8.0f;

	// 弾数（7〜13くらいが邪魔）
	const int count = 11;

	for (int i = 0; i < count; ++i) {
		float t = (count == 1) ? 0.5f : float(i) / float(count - 1);
		float x = (t - 0.5f) * width;

		Vector3 spawnPos = worldTransform_.translation_ + right * x;

		EnemyBullet* b = new EnemyBullet();
		Vector3 vel = baseDir * bulletSpeed_;
		b->Initialize(bulletModel_, spawnPos, vel);
		bullets_.push_back(b);
	}
}


void BarrageTurretEnemy::ShootBarrageCircle(const Vector3& playerPos) {

	// 1) プレイヤー方向（ゼロ割りガード）
	Vector3 toPlayer = playerPos - worldTransform_.translation_;
	Vector3 baseDir = (Length(toPlayer) < 1e-6f) ? Vector3{0.0f, 0.0f, -1.0f} : Normalized(toPlayer);

	// 2) baseDir と直交する right/up を作る（安定版）
	Vector3 tmpUp = {0.0f, 1.0f, 0.0f};
	if (std::fabs(Dot(baseDir, tmpUp)) > 0.9f) {
		tmpUp = {1.0f, 0.0f, 0.0f};
	}

	// ★Cross順をこの形にする（安定）
	Vector3 right = Normalized(Cross(baseDir, tmpUp));
	Vector3 up = Normalized(Cross(right, baseDir));

	// 3) 広がり
	const float spread = 0.6f;

	for (int i = 0; i < bulletCount_; ++i) {
		float angle = (2.0f * kPi / float(bulletCount_)) * float(i);

		Vector3 offset = right * std::cos(angle) + up * std::sin(angle);
		Vector3 dir = Normalized(baseDir + offset * spread);

		ShootOnce(dir);
	}
}

void BarrageTurretEnemy::ShootBarrageFan() {
	// 上方向基準で扇形（扇の中心を上方向にしたい）
	// ここは好みで forward を変えられる
	Vector3 forward = {0.0f, 1.0f, 0.0f};

	// fanAngle_ の範囲に bulletCount_ 発
	int count = (bulletCount_ < 3) ? 3 : bulletCount_;
	float half = fanAngle_ * 0.5f;

	for (int i = 0; i < count; ++i) {
		float t = (count == 1) ? 0.5f : float(i) / float(count - 1);
		float a = -half + (fanAngle_)*t;

		// forward(0,1) を回転（2D回転）
		float cs = std::cos(a);
		float sn = std::sin(a);

		Vector3 dir = {forward.x * cs - forward.y * sn, forward.x * sn + forward.y * cs, 0.0f};

		ShootOnce(dir);
	}
}

void BarrageTurretEnemy::Draw3D() {
	// 本体
	if (model_ && camera_) {
		model_->Draw(worldTransform_, *camera_);
	}
	// 弾
	for (EnemyBullet* enemyBullet : bullets_) {
		enemyBullet->Draw(*camera_);
	}
}

BarrageTurretEnemy::~BarrageTurretEnemy() {
	for (EnemyBullet* enemyBullet : bullets_) {
		delete enemyBullet;
	}
	bullets_.clear();
}
