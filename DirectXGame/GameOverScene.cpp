#include "GameOverScene.h"
#include "MatrixMath.h"
#include <cmath>

using namespace KamataEngine;

void GameOverScene::Initialize(GameManager* manager) {
	manager_ = manager;

	quadModel_ = Model::CreateFromOBJ("ui_quad");
	texGameOver_ = TextureManager::Load("gameover.png"); // ★ここ用意してね

	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();

	wt_.Initialize();
	wt_.translation_ = {0.0f, 0.0f, 0.0f};
	wt_.scale_ = {6.0f, 3.0f, 1.0f};
	wt_.rotation_ = {0.0f, 0.0f, 0.0f};

	// ui_quad が裏向きなら 180度回す（PauseMenuと同じ）
	const float kPi = 3.1415926535f;
	wt_.rotation_.y = kPi;

	WorldTransformUpdate(wt_);
	wt_.TransferMatrix();

	anim_ = 0.0f;
}

void GameOverScene::Finalize() {
	delete quadModel_;
	quadModel_ = nullptr;
}

void GameOverScene::Update() {
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		manager_->RequestChangeScene(SceneType::Title);
		return;
	}

	// ふわふわ（いらなければ消してOK）
	anim_ += 0.05f;
	wt_.translation_.y = std::sin(anim_) * 0.2f;

	WorldTransformUpdate(wt_);
	wt_.TransferMatrix();

	camera_.UpdateMatrix();
}

void GameOverScene::Draw() {
	if (!quadModel_) {
		return;
	}

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());
	quadModel_->Draw(wt_, camera_, texGameOver_);
	Model::PostDraw();
}
