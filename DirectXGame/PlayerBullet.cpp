#include "PlayerBullet.h"

void PlayerBullet::Initialize(Model* model, const Vector3& position,const Vector3 &velocity) {

	// NULLポインタチェック
	assert(model);

	// モデル
	player_bullet_model = model;

	// ワールド変換の初期化
	worldTransform_.Initialize();

	// 引数で受け取った初期座標をセット
	worldTransform_.translation_ = position;

	//引数で受け取った速度をメンバー変数に代入
	velocity_ = velocity;


	// ★ ここが重要：生成直後に一度ワールド行列を更新しておく
	WorldTransformUpdate(worldTransform_);

}

void PlayerBullet::Update() {

	// 座標を移動させる(1フレーム分の移動量を足しこむ)
	worldTransform_.translation_ += velocity_;

	//ワールドトランスフォームの更新
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	if (--dethTimer_ <= 0) {
		isDead_ = true;
	}

}

void PlayerBullet::Draw(const Camera& camera) {

	player_bullet_model->Draw(worldTransform_, camera);

}
