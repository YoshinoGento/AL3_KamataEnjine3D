#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
#include <vector>

using namespace KamataEngine;

class LockOnManager {
public:
	// 初期化
	void Initialize(uint32_t textureHandle);

	// 右クリックでロックオン
	void TryLockOn(const Vector2& mousePos, const std::vector<Enemy*>& enemies, const Camera& camera);

	// 2D描画
	void DrawMarkers(const Camera& camera);

	// ★ ロックしている敵一覧を取得
	std::vector<Enemy*> GetLockedEnemies() const;

	// ロック解除
	void Clear();

private:
	struct LockTarget {
		Enemy* enemy = nullptr;

		Sprite* tl = nullptr;
		Sprite* tr = nullptr;
		Sprite* bl = nullptr;
		Sprite* br = nullptr;
	};

	std::vector<LockTarget> targets_;
	uint32_t textureHandle_ = 0;
};
