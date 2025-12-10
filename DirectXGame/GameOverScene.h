#pragma once
#include "KamataEngine.h"
#include "Scene.h"
#include "SceneType.h"
using namespace KamataEngine;


class GameOverScene : public Scene {
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

	Sprite* background_ = nullptr; // フルスクリーン画像
	Sprite* pressKey_ = nullptr;   // 点滅テキスト

	int flashTimer_ = 0;
};
