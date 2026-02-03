// ClearScene.h
#pragma once
#include "GameManager.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "SkyDome.h" // 追加
using namespace KamataEngine;

class ClearScene : public IScene {
public:
	void Initialize(GameManager* manager) override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	GameManager* manager_ = nullptr;
	SkyDome sky_;
	Camera camera_;
	Model* quadModel_ = nullptr;
	WorldTransform wt_{};
	uint32_t texClear_ = 0;

private:
	void SetupTransform_();
};
