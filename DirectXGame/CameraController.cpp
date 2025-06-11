#include <algorithm>
#include "CameraController.h"
#include "Math.h"
#include "Player.h"




void CameraController::Initialize(Camera* camera) { 

	camera_ = camera; 
}

void CameraController::Update() {

	//追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	//追従対象とオフセットからカメラの座標を計算
	destination_ = targetWorldTransform.translation_ + targetOffset_;

	// 座標補間によりゆったり追従(数学関数追加)
	camera_->translation_ = Lerp(camera_->translation_, destination_, kInterpolationRate);
	//行列を更新する
	camera_->UpdateMatrix();


}

void CameraController::Reset() {

	//追従対象のワールドトランスフォームを参照
	const WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	//追従対象としてオフセットからカメラの座標を計算
	camera_->translation_ = targetWorldTransform.translation_ + targetOffset_;
}
