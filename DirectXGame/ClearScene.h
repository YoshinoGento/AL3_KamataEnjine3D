#pragma once
#include "Scene.h"
#include "SceneType.h"
#include "KamataEngine.h"

using namespace KamataEngine;

class ClearScene : public Scene {
public:
	void Initialize() override;
	void Update() override;
	void Draw3D() override {}
	void Draw2D() override;
	void Finalize() override;

	bool IsEnd() override { return isEnd_; }
	int NextScene() override { return nextScene_; }

private:
	bool isEnd_ = false;
	int nextScene_ = (int)SceneType::TITLE;

	Sprite* clearText_ = nullptr;
	Sprite* pressKey_ = nullptr;

	int flashTimer_ = 0; // ← 追加！点滅用タイマー
};
