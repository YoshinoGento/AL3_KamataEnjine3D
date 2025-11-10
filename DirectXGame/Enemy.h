#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Enemy {
public:
	// 初期化
	void Initialize(Model* model,  const Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

private:
	WorldTransform worldTransform_;
	Model* model_ = nullptr;
	uint32_t textureHandle_ = 0u;

};
