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

	// =========================
	// ① 基本の直線移動（start → target）
	// =========================
	Vector3 basePos = Lerp(start_, target_, t);

	// =========================
	// ② 回転量（最初は大きく、最後は0）
	// =========================
	float swirlPower = (1.0f - t);                      // 減衰
	float angle = t * std::numbers::pi_v<float> * 4.0f; // 2周回る

	// =========================
	// ③ 円運動オフセット（ファンネル感の核）
	// =========================
	Vector3 swirlOffset;
	swirlOffset.x = std::cos(angle) * controlOffset_.x * swirlPower;
	swirlOffset.y = controlOffset_.y * swirlPower;
	swirlOffset.z = std::sin(angle) * controlOffset_.z * swirlPower;

	// =========================
	// ④ 最終位置
	// =========================
	worldTransform_.translation_ = basePos + swirlOffset;

	// =========================
	// ⑤ 向き調整（進行方向）
	// =========================
	Vector3 velocity = worldTransform_.translation_ - prevPosition_;
	if (Length(velocity) > 0.001f) {
		Vector3 forward = Normalized(velocity);
		Vector3 defaultForward = {0, 0, 1};

		Vector3 axis = Cross(defaultForward, forward);
		float dot = Dot(defaultForward, forward);
		float angleRot = std::acos(std::clamp(dot, -1.0f, 1.0f));

		if (Length(axis) > 0.0001f) {
			axis = Normalized(axis);
			Matrix4x4 rotMat = MakeRotateAxisMatrix(axis, angleRot);
			worldTransform_.rotation_ = GetEulerFromMatrix(rotMat);
		}
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




