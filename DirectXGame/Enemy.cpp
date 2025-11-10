#include "Enemy.h"
#include "GameScene.h"

using namespace KamataEngine;


void Enemy::Initialize(Model* model, const Vector3& position) {

	assert(model);
	model_ = model;

	worldTransform_.Initialize();

	// 初期位置
	worldTransform_.translation_ = position;

}

void Enemy::Update() {

}
	
void Enemy::Draw() {

}



