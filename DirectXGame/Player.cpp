#include "Player.h"
#include "GameScene.h"
#include "ImageHlp.h"
#include "MapChipField.h"
#include "KamataEngine.h"
#include "Math.h"
#include <vector>  
#include <numbers>
using namespace KamataEngine;

void Player::Initialize(Model* model, Camera* camera, const Vector3& position) { worldTransform_.translation_ = position;
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
}
