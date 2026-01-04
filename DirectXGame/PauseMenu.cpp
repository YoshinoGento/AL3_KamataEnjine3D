#include "PauseMenu.h"
#include "MatrixMath.h" // WorldTransformUpdate 等を使うなら
#include <numbers>

void PauseMenu::Initialize(Camera* camera) {
	camera_ = camera;

	// 板ポリOBJ（共通）
	quadModel_ = Model::CreateFromOBJ("ui_quad"); // Resources/ui_quad.obj の想定

	// テクスチャ
	texTitle_ = TextureManager::Load("pause_title.png");

	texResume_ = TextureManager::Load("pause_resume.png");
	texResumeSel_ = TextureManager::Load("pause_resume_sel.png");

	texRestart_ = TextureManager::Load("pause_restart.png");
	texRestartSel_ = TextureManager::Load("pause_restart_sel.png");

	texToTitle_ = TextureManager::Load("pause_titlebtn.png");
	texToTitleSel_ = TextureManager::Load("pause_titlebtn_sel.png");

	wtTitle_.Initialize();
	wtResume_.Initialize();
	wtRestart_.Initialize();
	wtToTitle_.Initialize();
	
	SetupTransform_();
}

void PauseMenu::SetupTransform_() {
	const float z = -1.0f;

	wtTitle_.translation_ = {0.0f, 2.0f, z};
	wtResume_.translation_ = {0.0f, 0.8f, z};
	wtRestart_.translation_ = {0.0f, 0.0f, z};
	wtToTitle_.translation_ = {0.0f, -0.8f, z};

	wtTitle_.scale_ = {4.0f, 1.0f, 1.0f};
	wtResume_.scale_ = {3.5f, 0.7f, 1.0f};
	wtRestart_.scale_ = {3.5f, 0.7f, 1.0f};
	wtToTitle_.scale_ = {3.5f, 0.7f, 1.0f};

	// ★超重要：ui_quadが裏向きなら 180度回す（YでもXでもOK）
	const float kPi = 3.1415926535f;
	wtTitle_.rotation_.y = kPi;
	wtResume_.rotation_.y = kPi;
	wtRestart_.rotation_.y = kPi;
	wtToTitle_.rotation_.y = kPi;

	WorldTransformUpdate(wtTitle_);
	wtTitle_.TransferMatrix();
	WorldTransformUpdate(wtResume_);
	wtResume_.TransferMatrix();
	WorldTransformUpdate(wtRestart_);
	wtRestart_.TransferMatrix();
	WorldTransformUpdate(wtToTitle_);
	wtToTitle_.TransferMatrix();
}

PauseMenu::Result PauseMenu::Update() {
	if (!isOpen_)
		return Result::None;

	Input* in = Input::GetInstance();
	if (in->TriggerKey(DIK_UP))
		cursor_ = (cursor_ + 2) % 3;
	if (in->TriggerKey(DIK_DOWN))
		cursor_ = (cursor_ + 1) % 3;

	if (in->TriggerKey(DIK_RETURN)) {
		if (cursor_ == 0)
			return Result::Resume;
		if (cursor_ == 1)
			return Result::Restart;
		if (cursor_ == 2)
			return Result::ToTitle;
	}
	return Result::None;
}

void PauseMenu::Draw() {
	if (!isOpen_ || camera_ == nullptr || quadModel_ == nullptr) {
		return;
	}

	quadModel_->Draw(wtTitle_, *camera_, texTitle_);
	quadModel_->Draw(wtResume_, *camera_, (cursor_ == 0) ? texResumeSel_ : texResume_);
	quadModel_->Draw(wtRestart_, *camera_, (cursor_ == 1) ? texRestartSel_ : texRestart_);
	quadModel_->Draw(wtToTitle_, *camera_, (cursor_ == 2) ? texToTitleSel_ : texToTitle_);
}
