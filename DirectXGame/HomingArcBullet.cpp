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
}

void HomingArcBullet::Update() {
	time_ += 1.0f;
	float t = time_ / lifeTime_;

	if (t >= 1.0f) {
		isDead_ = true;
		return;
	}

	// ========= ① 制御点を1つだけ作る =========
	Vector3 mid = (start_ + target_) * 0.5f;

	// 上方向にだけ膨らませる（← 弧の正体）
	Vector3 control = mid + controlOffset_;

	// ========= ② 二次ベジェ曲線 =========
	Vector3 p0 = start_;
	Vector3 p1 = control;
	Vector3 p2 = target_;

	Vector3 a = Lerp(p0, p1, t);
	Vector3 b = Lerp(p1, p2, t);
	worldTransform_.translation_ = Lerp(a, b, t);

	// ========= ③ 向き調整 =========
	Vector3 velocity = worldTransform_.translation_ - prevPosition_;
	if (Length(velocity) > 0.0001f) {
		worldTransform_.rotation_ = LookRotation(velocity);
	}

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	prevPosition_ = worldTransform_.translation_;
}




void HomingArcBullet::Draw(const KamataEngine::Camera& camera) {
	if (!model_)
		return;
	model_->Draw(worldTransform_, camera);
}




