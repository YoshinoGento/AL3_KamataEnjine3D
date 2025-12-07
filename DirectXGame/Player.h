#pragma once
#include "Enemy.h"
#include "HomingArcBullet.h"
#include "KamataEngine.h"
#include "MatrixMath.h"
#include "MissilePartocle.h"
#include "PlayerBullet.h"
#include "Beam.h"
#include <list>

using namespace KamataEngine;

class Player {
public:
	void Initialize(Model* playerModel, Model* playerBulletModel, Camera* camera, const Vector3& position);
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
	Model* player_bullet_model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {0, 0, 0};
	std::list<PlayerBullet*> bullets_;
	std::list<HomingArcBullet*> arcBullets_;
	Input* input_ = nullptr;

	Enemy* enemy_ = nullptr;
	std::vector<Enemy*> lockedOnEnemies_;

	// 仮実装：プレイヤーのHP
	int hitPoint_ = 3;

	// 無敵時間（被弾直後の連続ヒット防止用）
	int invincibleTimer_ = 0;

	bool isFiringFanMissiles_ = false; // ファンネル弾発射中フラグ
	int fireInterval_ = 5;             // フレーム間隔
	int fireTimer_ = 0;                // タイマー
	int fireCount_ = 0;                // 発射済み弾数（最大6）
	Enemy* lockedEnemy_ = nullptr;     // ロックオンした敵
	std::list<MissilePartocle*> missileParticles_;
	std::list<Beam*> beams_;
};

