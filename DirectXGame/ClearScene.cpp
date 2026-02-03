// ClearScene.cpp
#include "ClearScene.h"
#include "MatrixMath.h"

void ClearScene::Initialize(GameManager* manager) {
	manager_ = manager;

	// スカイドーム初期化（GameSceneと同じモデルを使う）
	sky_.Initialize("FaceSkySphere", 3.0f);

	// ★変更："clear.obj" を読み込む
	clearModel_ = Model::CreateFromOBJ("clear");

	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();

	wt_.Initialize();
	SetupTransform_();

	// ★クリアBGM読み込み＆再生
	bgmHandle_ = Audio::GetInstance()->LoadWave("clear_bgm.wav");
	playHandle_ = Audio::GetInstance()->PlayWave(bgmHandle_, true, 0.03f);
}

void ClearScene::SetupTransform_() {
	wt_.translation_ = {0.0f, 0.0f, 0.0f};
	wt_.scale_ = {6.0f, 3.0f, 1.0f};

	// ★PauseMenuと同じ「裏向き対策」
	const float kPi = 3.1415926535f;
	wt_.rotation_.y = kPi;

	WorldTransformUpdate(wt_);
	wt_.TransferMatrix();
}

void ClearScene::Finalize() {
	// ※もし Model がエンジン管理なら delete しない方が安全な場合がある
	// いったん他と合わせるならそのままでOK
	// delete quadModel_;

	// ★BGM停止
	Audio::GetInstance()->StopWave(playHandle_);

	// モデル解放
	delete clearModel_;
	clearModel_ = nullptr;

	// スカイドーム終了処理
	sky_.Finalize();
}

void ClearScene::Update() {
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		manager_->RequestChangeScene(SceneType::Title);
		return;
	}
	// スカイドーム更新
	sky_.Update(camera_);
}

void ClearScene::Draw() {
	

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());
	// 3D描画
	// スカイドームを最初に描画
	sky_.Draw(camera_);

	// ★変更：3Dモデルとして描画
	if (clearModel_) {
		clearModel_->Draw(wt_, camera_);
	}
	Model::PostDraw();
}
