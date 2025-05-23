#include "Player.h"
#include "GameScene.h"
#include "ImageHlp.h"
#include "MapChipField.h"
#include "KamataEngine.h"
#include "Math.h"
#include <vector>  
#include <numbers>
#include "assert.h"
using namespace KamataEngine;

void Player::Initialize(Model* model_,Camera* camera, const Vector3& position) { 
	
	assert(model_);
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}

void Player::Update() {
	//移動入力
	//左右移動操作
	if (Input::GetInstance()->PushKey(DIK_RIGHT) ||
		Input::GetInstance()->PushKey(DIK_LEFT)) 
	{
		
		//左右加速
		Vector3 acceleration = {};
		if (Input::GetInstance()->PushKey(DIK_RIGHT)) {
		
			acceleration.x += kAcceleration;
		} else {
			
			acceleration.x -= kAcceleration;
		}
		//加速/減速
		velocity_ += acceleration;
	}

	//移動
	worldTransform_.translation_ += velocity_;

	//行列更新
	for (std::vector<WorldTransform*>& worldTransformPlayerLine : worldTransformPlayer_) {
		for (WorldTransform* worldTransformPlayer : worldTransformPlayerLine) {
			if (!worldTransformPlayer)
				continue;
			worldTransformPlayer->matWorld_ = MakeAffineMatrix(worldTransformPlayer->scale_, worldTransformPlayer->rotation_, worldTransformPlayer->translation_);
			// 定数バッファに転送する
			worldTransformPlayer->TransferMatrix();
		}
	}


}
