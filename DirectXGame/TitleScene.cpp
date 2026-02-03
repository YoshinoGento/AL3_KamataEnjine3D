#include "TitleScene.h"
#include "MatrixMath.h"

using namespace KamataEngine;

void TitleScene::Initialize(GameManager* manager) {
	manager_ = manager;

	// ★タイトル用OBJを用意してね（例：Resources/title.obj 相当）
	// CreateFromOBJ("title") で読み込めるようにしておく
	titleModel_ = Model::CreateFromOBJ("title");
	sky_.Initialize("FaceSkySphere", 3.0f);

	titleWT_.Initialize();
	titleWT_.translation_ = {0.0f, 0.0f, 0.0f};
	titleWT_.scale_ = {1.0f, 1.0f, 1.0f};

	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -15.0f};
	camera_.UpdateMatrix();

	rotY_ = 0.0f;

	// ★BGM読み込み＆再生（シングルトン経由）
	// LoadWaveなので .wav ファイルを用意してください
	bgmHandle_ = Audio::GetInstance()->LoadWave("title_bgm.wav");

	// ループ再生(true)、音量は0.5fくらい
	playHandle_ = Audio::GetInstance()->PlayWave(bgmHandle_, true, 0.03f);
}

void TitleScene::Finalize() {

	// ★BGM停止
	Audio::GetInstance()->StopWave(playHandle_);

	// スカイドーム終了処理
	sky_.Finalize();
	delete titleModel_;
	titleModel_ = nullptr;
}

void TitleScene::Update() {
	// Enterでゲーム開始
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		manager_->RequestChangeScene(SceneType::Game);
		return;
	}

	// スカイドーム更新（カメラに追従させる）
	sky_.Update(camera_);

	// ちょい演出：回転
	rotY_ += 0.01f;
	titleWT_.rotation_.y = rotY_;

	WorldTransformUpdate(titleWT_);
	titleWT_.TransferMatrix();

	camera_.UpdateMatrix();
}

void TitleScene::Draw() {
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	Model::PreDraw(dxCommon->GetCommandList());

	// 3D描画
	// スカイドームを最初に描画（背景）
	sky_.Draw(camera_);
	if (titleModel_) {
		titleModel_->Draw(titleWT_, camera_);
	}
	Model::PostDraw();
}
