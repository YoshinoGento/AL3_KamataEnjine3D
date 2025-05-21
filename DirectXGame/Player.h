#pragma once
#include "GameScene.h"
#include "KamataEngine.h"
#include "MapChipField.h" 


class Player {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);

private:
	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;
};