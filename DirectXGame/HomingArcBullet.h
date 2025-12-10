#pragma once
#include "KamataEngine.h"
#include "MatrixMath.h"
#include "MissilePartocle.h"
#include <list> 

using namespace KamataEngine;

class HomingArcBullet {
public:
	// モデル、開始位置、目標位置を受け取る初期化関数
	void Initialize(Model* model, const Vector3& start, const Vector3& target, const Vector3& controlOffset);

	// 更新処理（ベジェ曲線による放物線移動）
	void Update();

	// 描画
	void Draw(const Camera& camera);

	 // 現在位置取得
	Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	// 被弾などで自分を消す
	void OnHit() { isDead_ = true; }

	// 死亡判定
	bool IsDead() const { return isDead_; }

private:
	static const int32_t kLifeTime = 60 * 1;
	WorldTransform worldTransform_; // ワールド変換情報
	Model* model_ = nullptr;        // モデルポインタ
	uint32_t textureHandle_ = 0u;   // テクスチャハンドル
	Vector3 start_;                 // 弾の発射位置
	Vector3 target_;                // 目標位置
	float time_ = 0.0f;             // 経過時間
	float lifeTime_ = 180.0f;       // 寿命
	bool isDead_ = false;           // 死亡フラグ
	Vector3 control_;               // ベジェ曲線用の制御点（ランダム）
	Vector3 controlOffset_;         // ランダムオフセット

	// 🔽 追加（パーティクルの管理用）
	std::list<MissilePartocle*> missileParticles_;

	Vector3 prevPosition_;
};
