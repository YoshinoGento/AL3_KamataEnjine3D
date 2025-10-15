#include "Player.h"
#include "MatrixMath.h"
#include <cmath> 
#include <cassert>
#include <numbers>

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {

	    assert(model);
	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	// 初期位置
	worldTransform_.translation_ = position;

	// サイズ調整
	worldTransform_.scale_ = {2.0f, 2.0f, 2.0f};

	// 初期向き
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player ::Update() {}

void Player ::Draw() {

	// モデル描画
	model_->Draw(worldTransform_, *camera_);
}