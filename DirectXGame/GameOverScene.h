#pragma once
#include "GameManager.h"
#include "IScene.h"
#include "KamataEngine.h"
#include "SkyDome.h" // 追加

using namespace KamataEngine;

class GameOverScene : public IScene {
public:
	void Initialize(GameManager* manager) override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	GameManager* manager_ = nullptr;

	// ゲームオーバー画面用のスカイドーム
	SkyDome sky_;

	// ===== OBJ UI方式（ClearSceneと同じ） =====
	// ★変更：専用モデルを使う
	Camera camera_;
	Model* gameOverModel_ = nullptr;
	WorldTransform wt_{};

	float anim_ = 0.0f; // 演出（不要なら消してOK）

	// ★BGM用変数
	uint32_t bgmHandle_ = 0;
	uint32_t playHandle_ = 0;
};
