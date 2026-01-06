#include "HomingArcBullet.h"
#include "MatrixMath.h"
#include "KamataEngine.h"
#include <cassert>
#include <numbers>
#include <algorithm>

using namespace KamataEngine;


void HomingArcBullet::Initialize(Model* model, const Vector3& start, const Vector3& target, const Vector3& controlOffset) {
	assert(model);
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = start;

	start_ = start;
	target_ = target;
	controlOffset_ = controlOffset;

	time_ = 0.0f;
	lifeTime_ = kLifeTime;
	isDead_ = false;

	// ★これが大事：最初の「前フレーム位置」を確定
	prevPosition_ = start;

	// ★最初の行列も作って転送しておく（描画が安定）
	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}


void HomingArcBullet::Update() {
	// 移動前の位置を保存
	Vector3 oldPos = worldTransform_.translation_;

	time_ += 1.0f;
	float t = time_ / lifeTime_;
	if (t >= 1.0f) {
		isDead_ = true;
		return;
	}

	Vector3 mid = (start_ + target_) * 0.5f;
	Vector3 control = mid + controlOffset_;

	Vector3 a = Lerp(start_, control, t);
	Vector3 b = Lerp(control, target_, t);
	worldTransform_.translation_ = Lerp(a, b, t);

	// ★向き：今回の移動量で向ける（oldPos基準）
	Vector3 v = worldTransform_.translation_ - oldPos;
	if (Length(v) > 1e-4f) {
		worldTransform_.rotation_ = LookRotation(v);
	}

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	// prevPosition_ は oldPos でもいいけど、もう oldPos を使ってるなら不要
	prevPosition_ = worldTransform_.translation_;
}






void HomingArcBullet::Draw(const KamataEngine::Camera& camera) {
	if (!model_)
		return;
	model_->Draw(worldTransform_, camera);
}




