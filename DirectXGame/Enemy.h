#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include <list>

using namespace KamataEngine;

class Enemy {
public:
	// 初期化
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	// 更新
	void Update(Vector3 playerPos);

	void Attack(const Vector3& playerPos);

	// 描画
	void Draw();

	// 敵がダメージを受けたときの処理用
	void OnHit();

	~Enemy();

	// ★ 追加：敵のワールド座標
	Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	// ★ 追加：敵弾リスト（参照で返す）
	const std::vector<EnemyBullet*>& GetBullets() const { return bullets_; }

	// ★ 追加：敵の生存フラグ
	bool IsDead() const { return isDead_; }

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Camera* camera_ = nullptr;

	std::vector<EnemyBullet*> bullets_; // 敵弾リスト
	int attackTimer_ = 0;

	bool isDead_ = false; // 敵の生存フラグ
	int hp_ = 3;          // HP
};
