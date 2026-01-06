#include "PlayerBullet.h"
#include <cassert>

void PlayerBullet::Initialize(Model* model, const Vector3& position,const Vector3 &velocity) {

	// NULLポインタチェック
	assert(model);

	// モデル
	model_ = model;

	// テクスチャ読み込み


	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 引数で受け取った初期座標をセット
	worldTransform_.translation_ = position;

	//引数で受け取った速度をメンバー変数に代入
	velocity_ = velocity;
	
}

void PlayerBullet::Update() {

	// ① 移動
	worldTransform_.translation_ += velocity_;

	// ② 進行方向に向ける ★ここが重要
	if (Length(velocity_) > 0.0001f) {
		worldTransform_.rotation_ = LookRotation(Normalized(velocity_));
	}

	// ③ 行列更新
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	// ④ 寿命
	if (--dethTimer_ <= 0) {
		isDead_ = true;
	}
}

void PlayerBullet::Draw(const Camera& camera) {

	model_->Draw(worldTransform_, camera);

}
