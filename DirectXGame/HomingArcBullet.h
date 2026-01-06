#pragma once
#include "KamataEngine.h"
#include "MatrixMath.h"
#include <list>

using namespace KamataEngine;

/// <summary>
/// ファンネル型ホーミングミサイル
/// ・前半：円軌道
/// ・後半：ターゲットへ突入
/// </summary>
class HomingArcBullet {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="model">ミサイルモデル</param>
	/// <param name="start">発射位置</param>
	/// <param name="target">目標位置</param>
	/// <param name="controlOffset">
	/// 円軌道用オフセット
	/// X,Z : 回転半径
	/// Y   : 高さ
	/// </param>
	void Initialize(Model* model, const Vector3& start, const Vector3& target, const Vector3& controlOffset);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw(const Camera& camera);

	/// <summary>
	/// ワールド座標取得
	/// </summary>
	Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	/// <summary>
	/// 被弾などで強制破棄
	/// </summary>
	void OnHit() { isDead_ = true; }

	/// <summary>
	/// 生存判定
	/// </summary>
	bool IsDead() const { return isDead_; }

	float GetRadius() const { return radius_; }
	void OnCollision() { isDead_ = true; }


private:
	// ===============================
	// 定数
	// ===============================
	static constexpr int32_t kLifeTime = 60; // フレーム

	// ===============================
	// 基本データ
	// ===============================
	WorldTransform worldTransform_;
	Model* model_ = nullptr;

	Vector3 start_;
	Vector3 target_;
	Vector3 controlOffset_; // 円運動用オフセット

	// ===============================
	// 状態管理
	// ===============================
	float time_ = 0.0f;
	float lifeTime_ = kLifeTime;
	bool isDead_ = false;

	// ===============================
	// 向き計算用
	// ===============================
	Vector3 prevPosition_;

	// ミサイルは弾より大きめが当たりやすい
	float radius_ = 0.6f;  

	bool hasPrev_ = false;

	// ★モデルの前方向補正（まずは逆向き対策で180度）
	Vector3 modelForwardOffset_{0.0f, 3.14159265f, 0.0f};
};
