#include "SpawnDirector.h"
#include "Enemy.h"
#include <cmath>

void SpawnDirector::Initialize(const Settings& s) { s_ = s; }

float SpawnDirector::Length(const Vector3& v) { return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }

Vector3 SpawnDirector::Sub(const Vector3& a, const Vector3& b) { return Vector3{a.x - b.x, a.y - b.y, a.z - b.z}; }

bool SpawnDirector::CanSpawnAt(const Vector3& pos, const Vector3& playerPos, const std::vector<Enemy*>& enemies) const {
	// プレイヤーに近すぎるの禁止
	if (Length(Sub(pos, playerPos)) < s_.minPlayerDist) {
		return false;
	}

	// 既存の敵に近すぎるの禁止
	for (Enemy* e : enemies) {
		if (!e)
			continue;
		if (e->IsDead())
			continue;

		Vector3 ePos = e->GetWorldPosition();
		if (Length(Sub(pos, ePos)) < s_.minEnemyDist) {
			return false;
		}
	}

	return true;
}

bool SpawnDirector::FindNearestSafePos(const Vector3& desiredPos, const Vector3& playerPos, const std::vector<Enemy*>& enemies, Vector3& outPos) const {
	// まずそのままOKなら採用
	if (CanSpawnAt(desiredPos, playerPos, enemies)) {
		outPos = desiredPos;
		return true;
	}

	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
	const float kPi = 3.1415926535f;

	// desiredPos の周囲をランダムに探す
	for (int i = 0; i < s_.nearSearchTry; i++) {
		float r = dist01(rng_) * s_.nearSearchRadius;
		float theta = dist01(rng_) * 2.0f * kPi;

		Vector3 p = desiredPos;
		p.x += std::cos(theta) * r;
		p.z += std::sin(theta) * r;

		// Yはそのまま（必要なら上下も動かしてOK）
		if (CanSpawnAt(p, playerPos, enemies)) {
			outPos = p;
			return true;
		}
	}

	return false;
}

Vector3 SpawnDirector::GetSpawnPosRandomRing(const Vector3& playerPos, const std::vector<Enemy*>& enemies) const {
	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
	const float kPi = 3.1415926535f;

	for (int i = 0; i < s_.maxRetry; i++) {
		float r = s_.spawnMinR + (s_.spawnMaxR - s_.spawnMinR) * dist01(rng_);
		float theta = dist01(rng_) * 2.0f * kPi;

		Vector3 p{};
		p.x = playerPos.x + std::cos(theta) * r;
		p.z = playerPos.z + std::sin(theta) * r;
		p.y = playerPos.y + (-2.0f + 4.0f * dist01(rng_)); // -2〜+2あたり

		if (CanSpawnAt(p, playerPos, enemies)) {
			return p;
		}
	}

	// 最終手段：とにかく遠め
	return Vector3{playerPos.x, playerPos.y, playerPos.z + s_.spawnMaxR};
}
Vector3 SpawnDirector::GetSpawnPosFrontRing(const Vector3& playerPos, const std::vector<Enemy*>& enemies) const {
	std::uniform_real_distribution<float> dist01(0.0f, 1.0f);
	const float kPi = 3.1415926535f;

	// 「前方半円」だけ（-90度〜+90度）
	for (int i = 0; i < s_.maxRetry; i++) {
		float r = s_.spawnMinR + (s_.spawnMaxR - s_.spawnMinR) * dist01(rng_);
		float theta = (-0.5f * kPi) + dist01(rng_) * (kPi); // -pi/2 ～ +pi/2

		Vector3 p{};
		p.x = playerPos.x + std::cos(theta) * r;
		p.z = playerPos.z + std::sin(theta) * r;

		// ★前方に固定したいなら sin だけだと「前後」になるので、Zに補正を入れる
		// ここがポイント：必ず前に寄せる
		p.z = playerPos.z + std::abs(std::sin(theta)) * r;

		p.y = playerPos.y + (-2.0f + 4.0f * dist01(rng_));

		if (CanSpawnAt(p, playerPos, enemies)) {
			return p;
		}
	}

	// 最終手段：必ず前
	return Vector3{playerPos.x, playerPos.y, playerPos.z + s_.spawnMaxR};
}
