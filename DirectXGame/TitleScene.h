#pragma once
#include "GameManager.h"
#include "IScene.h"
#include "KamataEngine.h"

using namespace KamataEngine;

class TitleScene : public IScene {
public:
	void Initialize(GameManager* manager) override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	GameManager* manager_ = nullptr;

	Model* titleModel_ = nullptr;
	WorldTransform titleWT_;
	Camera camera_;

	float rotY_ = 0.0f; // ちょい演出（回転）
};
