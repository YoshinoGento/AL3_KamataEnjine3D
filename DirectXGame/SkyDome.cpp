#include "SkyDome.h"
#include <cassert>

void SkyDome::Initialize(const char* objName, float scale) {
	scale_ = scale;

	model_ = Model::CreateFromOBJ(objName);
	assert(model_);

	wt_.Initialize();
	wt_.scale_ = {scale_, scale_, scale_};

	WorldTransformUpdate(wt_);
	wt_.TransferMatrix();
}

void SkyDome::Update(const Camera& camera) {
	// 天球は「位置だけ」カメラに追従させる
	wt_.translation_ = camera.translation_;

	// スケールは常に固定（変えたいなら scale_ で）
	wt_.scale_ = {scale_, scale_, scale_};

	WorldTransformUpdate(wt_);
	wt_.TransferMatrix();
}

void SkyDome::Draw(const Camera& camera) {
	if (!model_) {
		return;
	}
	model_->Draw(wt_, camera);
}

void SkyDome::Finalize() {
	delete model_;
	model_ = nullptr;
}
