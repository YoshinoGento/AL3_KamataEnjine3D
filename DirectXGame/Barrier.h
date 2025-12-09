#pragma once
#include "KamataEngine.h"
#include "MatrixMath.h"

using namespace KamataEngine;

class Barrier {
public:
	void Initialize();                                              // モデル読み込みなど
	void Update();                                                  // クールタイム・耐久減少処理
	void Draw(const Camera& camera, const Vector3& playerPosition); // UI描画

	// 攻撃吸収（チャージもここから通知）
	void Absorb(float power);

	// 状態取得
	bool IsBroken() const;
	bool IsCoolingDown() const;

	// ビーム発射時（耐久全回復・復活）
	void OnBeamFired();

	// UI用割合（0.0〜1.0）
	float GetDurabilityRate() const;

	bool isActive_ = false; // バリアON/OFF

	void SetActive(bool active) { isActive_ = active; }
	bool IsActive() const { return isActive_; }

	void Reset();

	void Break();

	bool IsInCooldown()const;

private:
	float durability_ = kMaxDurability;
	float coolTime_ = 0.0f;
	bool isBroken_ = false;

	bool isInCooldown_ = false;
	int cooldownTimer_ = 0;
	const int kCooldownTime = 180; // 例：3秒

	Model* model_ = nullptr; // 半透明バリアモデル

	ObjectColor objectColor_; // ← これを追加

	// 定数
	static constexpr float kMaxDurability = 1500.0f;
	static constexpr float kCoolTime = 180.0f;

	Vector4 color_;
	WorldTransform worldTransform_;
};
