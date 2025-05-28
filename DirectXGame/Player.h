#pragma once
#include "GameScene.h"
#include "KamataEngine.h"
#include "MapChipField.h" 
using namespace KamataEngine;

class Player {
public:
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	void Update();
	void Draw();
	

private:
	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Vector3 velocity_ = {};


	KamataEngine::Camera* camera_;


	// モデル
	KamataEngine::Model* model = nullptr;

	static inline const float kAcceleration = 5.0f;

	uint32_t textureHandle = 0u;

};