#pragma once
#include "HomingArcBullet.h"
#include "KamataEngine.h"
#include "MatrixMath.h"
#include "PlayerBullet.h"
#include <list>
#include "Enemy.h"

using namespace KamataEngine;

class Player {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();
	void Draw();

	// ★ 追加：狙い点へ撃つAPI（GameSceneから呼ぶ）
	void FireToward(const Vector3& targetWorld);

	void OnHitByBeam(); //  被弾処理（未実装）

	int GetHP() const { return hitPoint_; } // 仮実装

	/// <summary>
	/// 攻撃
	/// </summary>

	void Attack();
	~Player();
	void SetEnemy(Enemy* enemy);

	/// <summary>
	/// プレイヤーが持っている弾リストへの参照を取得
	/// （読み取り専用のつもりなので const 参照を返す）
	/// </summary>
	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }

	// プレイヤーのワールド座標を取得
	const Vector3& GetWorldPosition() const { return worldTransform_.translation_; }

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {0, 0, 0};
	std::list<PlayerBullet*> bullets_;
	std::list<HomingArcBullet*> arcBullets_;
	Input* input_ = nullptr;

	Enemy* enemy_ = nullptr;
	std::vector<Enemy*> lockedOnEnemies_;

	Enemy* lockedEnemy_ = nullptr; // ロックオンした敵

	// 仮実装：プレイヤーのHP
	int hitPoint_ = 3;

	// 無敵時間（被弾直後の連続ヒット防止用）
	int invincibleTimer_ = 0;

};