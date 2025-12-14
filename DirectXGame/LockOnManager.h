#pragma once
#include "Enemy.h"
#include "MatrixMath.h"
#include <vector>

class LockOnManager {
public:
	void Initialize(uint32_t markerTexture);

	// ロックオン処理（右クリック）
	void TryLockOn(const Vector2& mousePos, const std::vector<Enemy*>& enemies, const Camera& camera);

	// 発射時にロックしている敵一覧を返す
	const std::vector<Enemy*>& GetLockedEnemies() const { return lockedEnemies_; }

	// 発射後ロック解除
	void Clear() { lockedEnemies_.clear(); }

	// 描画（敵の頭上にマーカーを描く）
	void DrawMarkers(const Camera& camera);

private:
	std::vector<Enemy*> lockedEnemies_;
	uint32_t markerTexture_ = 0;
	float lockRadius_ = 60.0f; // マウスとの距離
};
