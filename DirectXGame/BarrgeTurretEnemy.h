#pragma once
#include "Enemy.h"
#include "EnemyBullet.h"
#include <list>

// 固定砲台：プレイヤー狙い撃ちや回転弾幕を行う
class BarrageTurretEnemy : public Enemy {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position) override;
	void Update(const Vector3& playerPos) override;
	void Draw3D() override;
	~BarrageTurretEnemy() override;

	const std::list<EnemyBullet*>& GetBullets() const { return bullets_; }
	void SetBulletModel(Model* model) { bulletModel_ = model; }

private:
	enum class Pattern {
		Wait,   // 待機
		Spiral, // 回転弾幕（バラ撒き）
		Aimed   // 自機狙い（5WAY）
	};

	void UpdateBullets();

	// 攻撃関数
	// ★修正：プレイヤーの方向へ飛ばすために引数を追加
	void ShootSpiral(const Vector3& playerPos);
	void ShootAimed(const Vector3& playerPos);
	void ShootOnce(const Vector3& dir);

private:
	std::list<EnemyBullet*> bullets_;
	Model* bulletModel_ = nullptr;

	// 行動制御
	Pattern pattern_ = Pattern::Wait;
	int timer_ = 0; // 汎用タイマー

	// 弾のパラメータ
	float bulletSpeed_ = 0.25f;
	float spiralAngle_ = 0.0f; // 回転弾幕の今の角度

	// 移動・設置パラメータ
	float fixedZ_ = 0.0f;
	Vector3 velocity_ = {0.2f, 0.0f, 0.0f};
	bool enableMove_ = true;
	float minX_ = -20.0f;
	float maxX_ = 20.0f;
	float minY_ = 2.0f;
	float maxY_ = 12.0f;
};