#pragma once
//#include "HomingArcBullet.h"
#include "KamataEngine.h"
#include "MatrixMath.h"
#include "PlayerBullet.h"
#include <list>

using namespace KamataEngine;

class Player {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();
	void Draw();
	void Attack();
	~Player();

	Vector3 GetWorldPosition() const;

	// ★ 追加：プレイヤー弾リスト
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }

	// ★ 追加：被弾時に呼ぶ
	void OnHit();

	// ★ 追加：生存フラグ
	bool IsDead() const { return isDead_; }

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Camera* camera_ = nullptr;

	Vector3 velocity_ = {0, 0, 0};

	std::list<PlayerBullet*> bullets_;

	Input* input_ = nullptr;

	// ★ 追加：HP と死亡フラグ
	int hp_ = 5;
	bool isDead_ = false;
};
