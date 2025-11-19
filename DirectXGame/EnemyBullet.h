#pragma once
#include "KamataEngine.h"
#include "MatrixMath.h"

using namespace KamataEngine;

class EnemyBullet {

public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">モデル</param>
	/// <param name="position">初期座標</param>
	/// <param name="velocity">速度</param>
	void Initialize(Model* model, const Vector3& position, const Vector3& velocity);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	/// <param name="camera">カメラ</param>
	void Draw(const Camera& camera);

	bool IsDead() const { return isDead_; }


	// 追加
	Vector3 GetWorldPosition() const { return worldTransform_.translation_; }
	void OnCollision() { isDead_ = true; }

private:
	// ワールド変換データ
	WorldTransform worldTransform_;

	// モデル
	Model* model_ = nullptr;

	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;

	// 速度
	Vector3 velocity_;

	// 寿命＜frm＞
	static const int32_t kLifeTime = 60 * 5;

	// デスタイマー
	int32_t dethTimer_ = kLifeTime;

	// デスフラグ
	bool isDead_ = false;
};
