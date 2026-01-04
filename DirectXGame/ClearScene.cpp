// ClearScene.cpp
#include "ClearScene.h"
#include "MatrixMath.h"

void ClearScene::Initialize(GameManager* manager) {
	manager_ = manager;

	quadModel_ = Model::CreateFromOBJ("ui_quad");
	texClear_ = TextureManager::Load("clear.png");

	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -10.0f};
	camera_.UpdateMatrix();

	wt_.Initialize();
	SetupTransform_();
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
	quadModel_ = nullptr;
}

void ClearScene::Update() {
	if (Input::GetInstance()->TriggerKey(DIK_RETURN)) {
		manager_->RequestChangeScene(SceneType::Title);
		return;
	}
}

void ClearScene::Draw() {
	if (!quadModel_)
		return;

	DirectXCommon* dxCommon = DirectXCommon::GetInstance();
	Model::PreDraw(dxCommon->GetCommandList());
	quadModel_->Draw(wt_, camera_, texClear_);
	Model::PostDraw();
}
