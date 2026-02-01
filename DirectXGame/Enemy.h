#pragma once
#include "KamataEngine.h"
#include <vector>

using namespace KamataEngine;

class Enemy {
public:
	virtual ~Enemy() = default;

	virtual void Initialize(Model* model, Camera* camera, const Vector3& position);
	virtual void Update(const Vector3& playerPos);
	virtual bool CheckCollision(const Vector3& playerPos, float playerRadius);

	// ★修正：外部から座標を取得・設定するときも basePos_ を使う（移動ロジック用）
	Vector3 GetWorldPosition() const { return basePos_; }
	void SetWorldPosition(const Vector3& p) { basePos_ = p; }

	virtual void Draw3D();

	bool IsDead() const { return isDead_; }
	void Kill() { isDead_ = true; }

	void Damage(int amount);

	float GetRadius() const { return radius_; }

	void FaceTo(const Vector3& targetPos);

protected:
	bool isDead_ = false;

	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	Camera* camera_ = nullptr;

	float radius_ = 1.0f;
	int hp_ = 1;

	// ★追加：移動ロジック用の「本来の座標」
	// 演出でどれだけズレても、ここに戻ってくる
	Vector3 basePos_ = {0, 0, 0};

	// ★追加：ダメージ演出用
	Vector3 damageOffset_ = {0, 0, 0};      // 演出によるズレ
	Vector3 knockbackVelocity_ = {0, 0, 0}; // 吹き飛び速度
	int hitShakeTimer_ = 0;                 // 振動タイマー
};