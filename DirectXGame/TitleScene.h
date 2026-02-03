#pragma once
#include "GameManager.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "SkyDome.h" // 追加

using namespace KamataEngine;

class TitleScene : public IScene {
public:
	void Initialize(GameManager* manager) override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	GameManager* manager_ = nullptr;

	// タイトル画面用のスカイドーム
	SkyDome sky_;

	Model* titleModel_ = nullptr;
	WorldTransform titleWT_;
	Camera camera_;

	float rotY_ = 0.0f; // ちょい演出（回転）
};
