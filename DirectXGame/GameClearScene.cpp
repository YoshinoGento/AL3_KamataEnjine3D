#include "GameClearScene.h"
#include "Math.h"
#include <numbers>
#include "skydome.h"

GameClearScene::~GameClearScene() {

delete modelPlayer_;
	delete modelTitle_;

	// 02_13 12枚目
	delete fade_;
	delete modelSkydome_;
}

void GameClearScene::Initialize() {

	modelTitle_ = Model::CreateFromOBJ("GameClearFont", true);
	modelPlayer_ = Model::CreateFromOBJ("player");

	// カメラ初期化
	camera_.Initialize();

	const float kPlayerTitle = 2.0f;

	worldTransformTitle_.Initialize();

	worldTransformTitle_.scale_ = {kPlayerTitle, kPlayerTitle, kPlayerTitle};

	const float kPlayerScale = 10.0f;

	worldTransformPlayer_.Initialize();

	worldTransformPlayer_.scale_ = {kPlayerScale, kPlayerScale, kPlayerScale};

	worldTransformPlayer_.rotation_.y = 0.95f * std::numbers::pi_v<float>;

	worldTransformPlayer_.translation_.x = -2.0f;

	worldTransformPlayer_.translation_.y = -10.0f;

	// 02_13 12枚目
	fade_ = new Fade();
	fade_->Initialize();

	// 02_13 22枚目
	fade_->Start(Fade::Status::FadeIn, 1.0f);

	// 02_03 skydome生成
	skydome_ = new Skydome();

	// 初期化
	modelSkydome_ = Model::CreateFromOBJ("SkyDome", true);
	skydome_->Initialize(modelSkydome_, &camera_);
}

void GameClearScene::Update() {

	// 02_12 27枚目
	// 02_13 13枚目 27枚目で削除
	//	fade_->Update();

	// 02_13 27枚目
	switch (phase_) {
	case Phase::kFadeIn:
		fade_->Update();
		skydome_->Update();

		if (fade_->IsFinished()) {
			phase_ = Phase::kMain;
		}
		break;
	case Phase::kMain:
		skydome_->Update();
		if (Input::GetInstance()->PushKey(DIK_SPACE)) {
			fade_->Start(Fade::Status::FadeOut, 1.0f);
			phase_ = Phase::kFadeOut;
		}
		break;
	case Phase::kFadeOut:
		skydome_->Update();
		fade_->Update();
		if (fade_->IsFinished()) {
			finished_ = true;
		}
		break;
	}

	// 02_13 27枚目で↑のPhase::kMainブロックへ
	//	if (Input::GetInstance()->PushKey(DIK_SPACE)) {
	//	finished_ = true;
	//	}

	counter_ += 1.0f / 60.0f;
	counter_ = std::fmod(counter_, kTimeTitleMove);

	float angle = counter_ / kTimeTitleMove * 2.0f * std::numbers::pi_v<float>;

	worldTransformTitle_.translation_.y = std::sin(angle) + 10.0f;

	camera_.TransferMatrix();

	// アフィン変換～DirectXに転送(タイトル座標)
	WorldTransformUpdate(worldTransformTitle_);

	// アフィン変換～DirectXに転送（プレイヤー座標）
	WorldTransformUpdate(worldTransformPlayer_);
}

void GameClearScene::Draw() {

	DirectXCommon* dxCommon_ = DirectXCommon::GetInstance();
	// コマンドリストの取得
	ID3D12GraphicsCommandList* commandList = dxCommon_->GetCommandList();

	Model::PreDraw(commandList);
	skydome_->Draw();

	modelTitle_->Draw(worldTransformTitle_, camera_);
	modelPlayer_->Draw(worldTransformPlayer_, camera_);

	Model::PostDraw();

	// 02_13 13枚目
	fade_->Draw();
}
