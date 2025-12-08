#include "Skydome.h"
#include <cassert>

void Skydome::Initialize(Model* model, Camera* camera) {
	// NULLポインタチェック
	assert(model);
	// 引数として受け取ったデータをメンバ変数に記録する
	model_ = model;
	// ワールド変換の初期化
	worldTransform_.Initialize();
	// 引数の内容をメンバ変数に記録
	camera_ = camera;
}

void Skydome::Update() { worldTransform_.TransferMatrix(); }

void Skydome::Draw() {
	// 3Dモデル描画
	model_->Draw(worldTransform_, *camera_);
}