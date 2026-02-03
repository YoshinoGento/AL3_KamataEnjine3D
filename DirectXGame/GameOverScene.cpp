#include "GameOverScene.h"
#include "MatrixMath.h"
#include <cmath>

using namespace KamataEngine;

void GameOverScene::Initialize(GameManager* manager) {
	manager_ = manager; // 未使用警告(C4100)対策でメンバに代入

	// スカイドーム初期化（GameSceneと同じモデルを使う）
	sky_.Initialize("FaceSkySphere", 3.0f);

	// ★変更："gameover.obj" を読み込む
	gameOverModel_ = Model::CreateFromOBJ("gameover");

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

	// ★ゲームオーバーBGM読み込み＆再生
	bgmHandle_ = Audio::GetInstance()->LoadWave("gameover_bgm.wav");
	playHandle_ = Audio::GetInstance()->PlayWave(bgmHandle_, true, 0.03f);
}

void GameOverScene::Finalize() {

	// ★BGM停止
	Audio::GetInstance()->StopWave(playHandle_);

	// スカイドーム終了処理
	sky_.Finalize();
	
	// モデル解放
	delete gameOverModel_;
	gameOverModel_ = nullptr;
}

void GameOverScene::Update() {
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		manager_->RequestChangeScene(SceneType::Title);
		return;
	}
	// スカイドーム更新
	sky_.Update(camera_);
	// ふわふわ（いらなければ消してOK）
	anim_ += 0.05f;
	wt_.translation_.y = std::sin(anim_) * 0.2f;

	WorldTransformUpdate(wt_);
	wt_.TransferMatrix();

	camera_.UpdateMatrix();
}

void GameOverScene::Draw() {


	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());
	// 3D描画
	// スカイドームを最初に描画
	sky_.Draw(camera_);
	// ★変更：3Dモデルとして描画
	if (gameOverModel_) {
		gameOverModel_->Draw(wt_, camera_);
	}

	Model::PostDraw();
}
