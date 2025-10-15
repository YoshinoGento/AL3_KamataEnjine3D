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
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f; // Y軸回転調整
}

void Player ::Update() {}

void Player ::Draw() {

	// モデル描画
	model_->Draw(worldTransform_, *camera_);
}