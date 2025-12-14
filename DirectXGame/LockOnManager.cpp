#include "LockOnManager.h"


void LockOnManager::Initialize(uint32_t markerTexture) { markerTexture_ = markerTexture; }

void LockOnManager::TryLockOn(const Vector2& mousePos, const std::vector<Enemy*>& enemies, const Camera& camera) {
	Enemy* best = nullptr;
	float bestDist = lockRadius_;

	for (Enemy* enemy : enemies) {
		Vector3 pos = enemy->GetWorldPosition();
		Vector2 screen = WorldToScreen(pos, camera);

		float dist = Length(mousePos - screen);

		if (dist < bestDist) {
			bestDist = dist;
			best = enemy;
		}
	}

	if (best) {
		// すでにロック済みでなければ追加
		if (std::find(lockedEnemies_.begin(), lockedEnemies_.end(), best) == lockedEnemies_.end()) {
			lockedEnemies_.push_back(best);
		}
	}
}

void LockOnManager::DrawMarkers(const Camera& camera) {
	for (Enemy* enemy : lockedEnemies_) {
		Vector3 pos = enemy->GetWorldPosition();
		Vector2 screen = WorldToScreen(pos, camera);

		screen.y -= 30; // 少し上に描画

		Sprite::Draw(markerTexture_, screen.x, screen.y, 32, 32);
	}
}
