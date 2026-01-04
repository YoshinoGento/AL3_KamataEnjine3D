// ClearScene.h
#pragma once
#include "GameManager.h"
#include "IScene.h"
#include "KamataEngine.h"
using namespace KamataEngine;

class ClearScene : public IScene {
public:
	void Initialize(GameManager* manager) override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	GameManager* manager_ = nullptr;

	Camera camera_;
	Model* quadModel_ = nullptr;
	WorldTransform wt_{};
	uint32_t texClear_ = 0;

private:
	void SetupTransform_();
};
