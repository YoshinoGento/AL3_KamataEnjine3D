#pragma once
#include "Enemy.h"
#include "HomingArcBullet.h"
#include "KamataEngine.h"
#include "LockOnManager.h"
#include "MatrixMath.h"
#include "PlayerBullet.h"
#include <list>
#include <vector>

using namespace KamataEngine;

class Player {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position, uint32_t lockonTexture);
	void Update();
	void Draw3D();
	void Draw2D();
	void Attack();
	~Player();

	void OnHit();

	Vector3 GetWorldPosition() const { return worldTransform_.translation_; }

	void OnHitByBeam();

	// エラーが出ていた箇所：radius_ を返します
	float GetRadius() const { return radius_; }

	void SetEnemies(const std::vector<Enemy*>* enemies);

	void SetAimPlaneZ(float z) { aimPlaneZ_ = z; }

	// 狙う奥行き
	float aimPlaneZ_ = 40.0f;

	const std::list<PlayerBullet*>& GetBullets() const { return bullets_; }

	const std::list<HomingArcBullet*>& GetArcBullets() const { return arcBullets_; }

	bool IsDead() const { return hitPoint_ <= 0; }

	void SetBulletModel(Model* model) { playerBulletModel_ = model; }
	void SetMissileModel(Model* model) { playerMissileModel_ = model; }

	void SetShotSE(uint32_t se) { seShot_ = se; }
	void SetMissileSE(uint32_t se) { seMissile_ = se; }

private:
	// マウス位置から平面上の座標を計算するヘルパー関数
	Vector3 CalcMouseHitOnZPlane(const Vector2& mouse, float planeZ) const;

	// 敵ポインタが現在の敵リストに存在するか確認する関数
	bool IsAlivePointerInEnemies(Enemy* enemy) const;

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;
	Camera* camera_ = nullptr;
	Vector3 velocity_ = {0, 0, 0};
	std::list<PlayerBullet*> bullets_;
	std::list<HomingArcBullet*> arcBullets_;
	Input* input_ = nullptr;
	const std::vector<Enemy*>* enemies_ = nullptr;
	std::vector<Enemy*> lockedOnEnemies_;

	// ロックオン管理
	LockOnManager lolckOn_;

	// 弾モデル
	Model* playerBulletModel_ = nullptr;
	Model* playerMissileModel_ = nullptr;

	// SE
	uint32_t seShot_ = 0;
	uint32_t seMissile_ = 0;

	// 無敵時間
	int invincibleTimer_ = 0;

	// HP
	int hitPoint_ = 10;

	// ★修正：ここに確実に定義
	float radius_ = 1.0f;

	// HP表示用
	uint32_t hpTextureHandle_ = 0;
	std::vector<Sprite*> hpIcons_;
};