#pragma once
#include "Enemy.h"
#include "EnemyBullet.h"
#include <list>

class ShooterEnemy : public Enemy {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position) override;
	void Update(const Vector3& playerPos) override;
	void Draw3D() override;

	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }

private:
	// ===== 行動状態 =====
	enum class State {
		Shoot, // 6発撃つ
		Move   // 別の場所へ移動
	};

	// ===== 内部処理 =====
	void UpdateShoot(const Vector3& playerPos);
	void UpdateMove();
	void Shoot(const Vector3& playerPos);
	void BeginMove();

private:
	State state_ = State::Shoot;

	// ---- 敵弾 ----
	std::list<EnemyBullet*> bullets_;
	Model* bulletModel_ = nullptr;
	float bulletSpeed_ = 0.35f;

	// ---- 射撃制御 ----
	int shotsLeft_ = 6;
	int shotTimer_ = 0;
	int shotInterval_ = 15;

	// ---- 移動制御 ----
	Vector3 moveStart_{};
	Vector3 moveTarget_{};
	float moveT_ = 0.0f;
	float moveSpeed_ = 0.02f;

	// ---- 移動範囲 ----
	float minX_ = -6.0f;
	float maxX_ = 6.0f;
	float minY_ = -2.0f;
	float maxY_ = 2.0f;
	float fixedZ_ = 10.0f;
};
