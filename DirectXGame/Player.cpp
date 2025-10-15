#include "Player.h"

#include <algorithm>
#include <cassert>
#include <numbers>


using namespace KamataEngine;

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) {
	assert(model);
	model_ = model;
	camera_ = camera;

	// ワールド変換初期化
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f; // Y軸回転で向きを調整



}

void Player::Update() {

	


}

void Player::Drow() {

	

}
