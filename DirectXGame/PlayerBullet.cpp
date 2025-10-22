#include "PlayerBullet.h"

void PlayerBullet::Initialize(Model* model, const Vector3& position, const Vector3& velocity) {

	// NULLポインタチェック
	assert(model);

	// モデル
	model_ = model;

	// テクスチャ読み込み
	textureHandle_ = TextureManager::Load("uvChecker.png");

	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 引数で受け取った初期座標をセット
	worldTransform_.translation_ = position;

	//引数で受け取った速度をメンバー変数に代入
	velocity_ = {0.0f, 0.0f, 0.2f};

}

void PlayerBullet::Update() {

	// 座標を移動させる(1フレーム分の移動量を足しこむ)
	worldTransform_.translation_ += velocity_;

	//ワールドトランスフォームの更新
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

}

void PlayerBullet::Draw(const Camera& camera) {

	model_->Draw(worldTransform_, camera, textureHandle_);

}
