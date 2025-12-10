#pragma once
#include "KamataEngine.h"
#include "Scene.h"
#include "SceneType.h"

using namespace KamataEngine;

class TitleScene : public Scene {
public:
	void Initialize() override;
	void Update() override;
	void Draw3D() override {} // タイトルは3Dなし
	void Draw2D() override;   // スプライトだけ
	void Finalize() override;

	bool IsEnd() override { return isEnd_; }
	int NextScene() override { return nextScene_; }

private:
	bool isEnd_ = false;
	int nextScene_ = 0;

	Sprite* title_ = nullptr;
	Sprite* pressKey_ = nullptr;

	int flashTimer_ = 0;

	uint32_t bgmDataHandle_ = 0;
	uint32_t bgmVoiceHandle_ = 0;
};
