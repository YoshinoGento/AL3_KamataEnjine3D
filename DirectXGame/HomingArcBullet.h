#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
#include "MatrixMath.h"

using namespace KamataEngine;

class HomingArcBullet {
public:
	// モデル、開始位置、目標位置を受け取る初期化関数
	void Initialize(Model* model, const Vector3& start, const Vector3& target);

	// 更新処理（ベジェ曲線による放物線移動）
	void Update();

	// 描画
	void Draw(const Camera& camera);

	// 死亡判定
	bool IsDead() const { return isDead_; }

	// ワールド座標を取得
	Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

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
	
    
	
};
