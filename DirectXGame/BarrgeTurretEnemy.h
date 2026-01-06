#pragma once
#include "Enemy.h"
#include "EnemyBullet.h"
#include <list>

// 固定砲台：プレイヤー狙いではなく円形/扇形に弾をばらまく
class BarrageTurretEnemy : public Enemy {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position) override;
	void Update(const Vector3& playerPos) override; // playerPosは使わなくてOK
	void Draw3D() override;
	~BarrageTurretEnemy() override;

	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }

	 void SetBulletModel(Model* model) { bulletModel_ = model; }

private:
	enum class Pattern {
		Circle, // 全方向
		Fan     // 扇形
	};

	void UpdateBullets();
	void ShootBarrageCircle(const Vector3& playerPos);
	void ShootBarrageFan();
	void ShootOnce(const Vector3& dir);
	void ShootBarrageWall(const Vector3& playerPos);

private:
	// 弾
	std::list<EnemyBullet*> bullets_;
	Model* bulletModel_ = nullptr;

	// 発射制御
	int shotTimer_ = 0;
	int shotInterval_ = 90; // 90f = 1.5秒（60fps想定）

	// 弾のパラメータ
	float bulletSpeed_ = 0.28f;
	int bulletCount_ = 16; // 円形の弾数
	Pattern pattern_ = Pattern::Circle;

	// 扇形用（ラジアン）
	float fanAngle_ = 1.2f; // 約68度くらい（左右合計）

	// ---- 移動（反射） ----
	Vector3 velocity_{0.06f, 0.04f, 0.0f}; // 斜めに動く（好きに調整）
	float minX_ = -6.0f;
	float maxX_ = 6.0f;
	float minY_ = -3.0f;
	float maxY_ = 3.0f;
	float fixedZ_ = 10.0f;   // 敵のZ固定
	bool enableMove_ = true; // 砲台＝false、動く敵＝true にできる
};
