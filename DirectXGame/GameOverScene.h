#pragma once
#include "GameManager.h"
#include "IScene.h"
#include "KamataEngine.h"

using namespace KamataEngine;

class GameOverScene : public IScene {
public:
	void Initialize(GameManager* manager) override;
	void Update() override;
	void Draw() override;
	void Finalize() override;

private:
	GameManager* manager_ = nullptr;

	// ===== OBJ UI方式（ClearSceneと同じ） =====
	Camera camera_;
	Model* quadModel_ = nullptr;
	WorldTransform wt_{};
	uint32_t texGameOver_ = 0;

	float anim_ = 0.0f; // 演出（不要なら消してOK）
};
