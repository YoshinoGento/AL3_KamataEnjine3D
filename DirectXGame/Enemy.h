#pragma once
#include "KamataEngine.h"
#include "EnemyBullet.h"
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

	~Enemy();

	Vector3 GetWorldPosition() const;

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Camera* camera_ = nullptr;

	
    std::vector<EnemyBullet*> bullets_; // 敵弾リスト
	int attackTimer_ = 0;
};
