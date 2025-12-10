#pragma once
#include "KamataEngine.h"
#include "MatrixMath.h"

using namespace KamataEngine;

class Player;

class EnemyBullet {
public:
	void Initialize(Model* model, const Vector3& position, const Vector3& velocity);
	void Update();
	void Draw(const Camera& camera);
	bool IsDead() const { return isDead_; }

	void SetPlayer(Player* player) { player_ = player; }

	// 敵弾のワールド座標を取得
	const Vector3& GetWorldPosition() const { return worldTransform_.translation_; }

	bool IsHitByPlayerBullet(const Vector3& bulletPosition);

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	//uint32_t textureHandle_ = 0u;
	Vector3 velocity_;
	static const int32_t kLifeTime = 60 * 5;
	int32_t dethTimer_ = kLifeTime;
	bool isDead_ = false;

	Player* player_ = nullptr;

	// 弾の速度
	const float kBulletSpeed = 0.15f;

	// 代入変数
	float t = 0.2f;
};
