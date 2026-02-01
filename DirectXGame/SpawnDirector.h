#pragma once
#include "KamataEngine.h"
#include <random>
#include <vector>


using namespace KamataEngine;

class Enemy;

class SpawnDirector {
public:
	struct Settings {
		float minEnemyDist = 3.0f;   // 敵同士の最低距離
		float minPlayerDist = 12.0f; // プレイヤーから最低距離

		float spawnMinR = 15.0f; // ランダムスポーン半径 min
		float spawnMaxR = 30.0f; // ランダムスポーン半径 max

		int maxRetry = 50;

		// posが危険だった時に「pos付近でズラす」探索
		float nearSearchRadius = 6.0f;
		int nearSearchTry = 30;
	};

public:
	void Initialize(const Settings& s);

	// そこに湧いてOK？
	bool CanSpawnAt(const Vector3& pos, const Vector3& playerPos, const std::vector<Enemy*>& enemies) const;

	// pos付近で安全な位置を探す（あればtrue）
	bool FindNearestSafePos(const Vector3& desiredPos, const Vector3& playerPos, const std::vector<Enemy*>& enemies, Vector3& outPos) const;

	// どうしてもダメなら「プレイヤー周りのリング」に出す
	Vector3 GetSpawnPosRandomRing(const Vector3& playerPos, const std::vector<Enemy*>& enemies) const;

	Vector3 GetSpawnPosFrontRing(const Vector3& playerPos, const std::vector<Enemy*>& enemies) const;


private:
	Settings s_{};

	mutable std::mt19937 rng_{std::random_device{}()};

private:
	static float Length(const Vector3& v);
	static Vector3 Sub(const Vector3& a, const Vector3& b);
};
