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

	void SetBulletModel(Model* model) { bulletModel_ = model; }

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
	float moveTimer_ = 0.0f;
	float fixedZ_ = 0.0f; // Z座標を固定するための変数

	// ★移動にかける時間（エラー回避用）
	float moveDuration_ = 2.0f;

	// ★移動範囲（エラー回避用）
	float minX_ = -18.0f;
	float maxX_ = 18.0f;
	float minY_ = 2.0f;
	float maxY_ = 12.0f;
};