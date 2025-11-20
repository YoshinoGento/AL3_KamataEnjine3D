#pragma once
#include "EnemyBullet.h"
#include "KamataEngine.h"
#include "MatrixMath.h"


using namespace KamataEngine;

class Enemy {
public:
	// 初期化：bossBasePosition を中心に部位を配置する
	void Initialize(const Vector3& bossBasePosition);

	Vector3 GetWorldPosition() const;

	// 描画
	void Draw(const Camera& camera);

	///< summary>
	/// 弾の位置を受け取って、どれかの部位に当たったらtrueを返す
	/// （当たった部位のHPを減らして、0以下なら部位破壊扱い）
	///</summary>
	bool CheckHit(const Vector3& bulletPosition);

private:
	struct BodyPart {
		Vector3 centerPosition; // 矩形の中心座標（ワールド）
		Vector3 boxSize;        // 矩形のサイズ（X,Y,Z 幅）
		int32_t hitPoint;       // 耐久値
		bool isDestroyed;       // 破壊済みフラグ
	};

	static const int kBodyPartCount = 3;
	BodyPart bodyParts_[kBodyPartCount];

	// 各部位の見た目用のワールド変換
	WorldTransform worldTransforms_[kBodyPartCount];

	// 可視化用モデル（暫定で "player" モデルを流用）
	Model* model_ = nullptr;
};
