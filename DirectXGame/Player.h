#pragma once
#include "GameScene.h"
#include "KamataEngine.h"
#include "MapChipField.h" 


class Player {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();

private:
	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	Vector3 velocity_ = {};

	// 3Dブロックモデル
	KamataEngine::Model* player_model_ = nullptr;
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformPlayer_;

	static inline const float kAcceleration = 5.0f;
};