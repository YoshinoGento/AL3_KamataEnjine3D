#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
using namespace KamataEngine;

class TacklerEnemy : public Enemy {
	void Initialize(Model* model, Camera* camera, const Vector3& position) override;
	void Update(const Vector3& playerPos) override;
	void Draw3D() override;

private:
	Vector3 moveDir_{0, 0, 1};  // 今向いている方向
	float speed_ = 0.25f;       // タックル速度
	float homingPower_ = 0.05f; // ← タックル用ホーミング（弾より強め）

	// 1秒あたり何度曲がれるか（小さいほど弱追尾）
	float turnRateDeg_ = 90.0f;

	// 消える判定用
	Vector3 spawnPos_{};
	float maxTravelDistance_ = 80.0f; // スポーン地点からこの距離以上進んだら消す

	bool lockedOn_ = false;//最初の一回だけねらいをきめる
	bool passedPlayer_ = false;      // 通り過ぎたか
	float despawnAfterPass_ = 10.0f; // 通り過ぎてからこの距離進んだら消える

	float homingTime_ = 0.20f; // ホーミングする時間
	float homingTimer_ = 0.0f; // 経過

};
