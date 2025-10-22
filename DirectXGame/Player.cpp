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

void Player ::Update() {

     // Input インスタンス取得
	auto* input = KamataEngine::Input::GetInstance();

	float moveSpeed = 110.1f;

	// 上下左右移動
	if (input->PushKey(DIK_W)) { // 上
		worldTransform_.translation_.y += moveSpeed;
	}
	if (input->PushKey(DIK_S)) { // 下
		worldTransform_.translation_.y -= moveSpeed;
	}
	if (input->PushKey(DIK_A)) { // 左
		worldTransform_.translation_.x -= moveSpeed;
	}
	if (input->PushKey(DIK_D)) { // 右
		worldTransform_.translation_.x += moveSpeed;
	}

	// 移動後の行列更新
	worldTransform_.TransferMatrix();

}

void Player ::Draw() {

	// モデル描画
	model_->Draw(worldTransform_, *camera_);
}