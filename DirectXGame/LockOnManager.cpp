#include "LockOnManager.h"
#include "MatrixMath.h"

void LockOnManager::Initialize(uint32_t textureHandle) {
	textureHandle_ = textureHandle;

	// ← ここでは Sprite を作らない
}



void LockOnManager::TryLockOn(const Vector2& mousePos, const std::vector<Enemy*>& enemies, const Camera& camera) {

	const float lockRadius = 60.0f;

	Enemy* bestEnemy = nullptr;
	float bestDist = lockRadius;

	for (Enemy* enemy : enemies) {
		Vector2 screen = WorldToScreen(enemy->GetWorldPosition(), camera, WinApp::kWindowWidth, WinApp::kWindowHeight);
		float dist = Length(mousePos - screen);
		if (dist < bestDist) {
			bestDist = dist;
			bestEnemy = enemy;
		}
	}

	if (!bestEnemy)
		return;

	// すでにロック済みなら無視
	for (auto& t : targets_) {
		if (t.enemy == bestEnemy)
			return;
	}

	LockTarget target{};
	target.enemy = bestEnemy;

	target.tl = Sprite::Create(textureHandle_, {0, 0});
	target.tr = Sprite::Create(textureHandle_, {0, 0});
	target.bl = Sprite::Create(textureHandle_, {0, 0});
	target.br = Sprite::Create(textureHandle_, {0, 0});

	target.tr->SetIsFlipX(true);
	target.bl->SetIsFlipY(true);
	target.br->SetIsFlipX(true);
	target.br->SetIsFlipY(true);

	targets_.push_back(target);
}



void LockOnManager::DrawMarkers(const Camera& camera) {

	// ① 死んだ敵を targets_ から消す
	for (auto it = targets_.begin(); it != targets_.end();) {
		if (!it->enemy || it->enemy->IsDead()) {
			delete it->tl;
			delete it->tr;
			delete it->bl;
			delete it->br;
			it = targets_.erase(it);
		} else {
			++it;
		}
	}

	// ② 残ってるターゲットを描画
	for (auto& t : targets_) {
		Vector2 center = 
			WorldToScreen(
				t.enemy->GetWorldPosition(),
				camera,
				WinApp::kWindowWidth,
				WinApp::kWindowHeight
			);

		float size = 48.0f;
		Vector2 half{size * 0.5f, size * 0.5f};

		t.tl->SetPosition(center + Vector2{-half.x, -half.y});
		t.tr->SetPosition(center + Vector2{half.x, -half.y});
		t.bl->SetPosition(center + Vector2{-half.x, half.y});
		t.br->SetPosition(center + Vector2{half.x, half.y});

		t.tl->Draw();
		t.tr->Draw();
		t.bl->Draw();
		t.br->Draw();
	}
}


std::vector<Enemy*> LockOnManager::GetLockedEnemies() const {
	std::vector<Enemy*> result;
	for (const auto& t : targets_) {
		result.push_back(t.enemy);
	}
	return result;
}

void LockOnManager::Clear() {
	for (auto& t : targets_) {
		delete t.tl;
		delete t.tr;
		delete t.bl;
		delete t.br;
	}
	targets_.clear();
}
