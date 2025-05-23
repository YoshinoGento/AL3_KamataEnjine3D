#include "skydome.h"
#include "GameScene.h"

void skydome::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {
	// NULLポインタチェック
	assert(model);

	// Mode引数の内容をメンバー関数に記録
	model = model;

	worldTransform.Initialize();

	camera = camera;
}

void skydome::Update() {

	// アフィン変換行列
	worldTransform.matWorld = math.MakeAffineMatrix(worldTransform.scale, worldTransform.rotation, worldTransform.translation);

	// 行列を定数バッファに転送
	worldTransform.TransferMatrix();
}

void skydome::Draw() {

	// ３Ｄモデルを描画
	model->Draw(worldTransform, *camera);
}