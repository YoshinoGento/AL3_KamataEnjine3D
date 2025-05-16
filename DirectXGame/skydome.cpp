#include "skydome.h"
#include "GameScene.h"

void skydome::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {


}

void skydome::Update() {


}

void skydome::Draw() {
	model_->Draw(worldTransform_, *camera_);

}

