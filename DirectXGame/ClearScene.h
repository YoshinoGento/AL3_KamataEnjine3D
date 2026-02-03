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
	// ★変更：板ポリ+画像ではなく、専用モデルを使う
	Model* clearModel_ = nullptr;
	WorldTransform wt_{};
	uint32_t texClear_ = 0;
	// ★BGM用変数
	uint32_t bgmHandle_ = 0;
	uint32_t playHandle_ = 0;



private:
	void SetupTransform_();
};
