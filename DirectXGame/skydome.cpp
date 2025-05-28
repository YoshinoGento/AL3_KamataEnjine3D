#include "skydome.h"
#include "GameScene.h"

void skydome::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
	// NULLポインタチェック
	assert(model);

	// Mode引数の内容をメンバー関数に記録
	model = model;

	worldTransform_.Initialize();

	camera = camera;
}

void skydome::Update() {

	// 行列を定数バッファに転送
	worldTransform_.TransferMatrix();
}

void skydome::Draw() {

	// ３Ｄモデルを描画
	model_->Draw(worldTransform_, *camera_);
}