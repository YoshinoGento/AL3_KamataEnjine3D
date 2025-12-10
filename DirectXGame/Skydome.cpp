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

	// 色乗算用の初期化（通常は白＝無加工）
	color_.Initialize();
	color_.SetColor(Vector4{1.0f, 1.0f, 1.0f, 1.0f});
}

void Skydome::Update() { worldTransform_.TransferMatrix(); }

void Skydome::Draw() {
	// 3Dモデル描画（色乗算つき）
	model_->Draw(worldTransform_, *camera_, &color_);
}

void Skydome::SetTintColor(const Vector4& color) {
	// ティントカラーをそのまま乗算色として使う
	color_.SetColor(color);
}
