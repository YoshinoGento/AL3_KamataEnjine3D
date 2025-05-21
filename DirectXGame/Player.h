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
};