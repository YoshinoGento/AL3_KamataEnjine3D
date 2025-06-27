#include <cassert>
#include <numbers>
#include "Enemy.h"
#include "Math.h"

void Enemy::Initialize(Model* model, Camera* camera, const Vector3& position) {

	//NULLチェック
	assert(model);

	// 02_09 7枚目
	model_ = model;

	// 02_09 7枚目
	camera_ = camera;

	// 02_09 7枚目
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	// 02_09 7枚目 角度調整
	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;

	// 02_09 16枚目 速度を設定する
	velocity_ = {-kWalkSpeed, 0, 0};

}

void Enemy::Update() {

	// 02_09 16枚目 移動
	worldTransform_.translation_ += velocity_;

	// 02_09 スライド8枚目 ワールド行列更新
	WorldTransformUpdate(worldTransform_);

}

void Enemy::Draw() {

	//02_09 スライド9枚目 モデル描画
	model_->Draw(worldTransform_, *camera_);

}
