#include "HomingArcBullet.h"
#include "MatrixMath.h"
#include "KamataEngine.h"
#include <cassert>

using namespace KamataEngine;


void HomingArcBullet::Initialize(Model* model, const Vector3& position, const Vector3& initialVelocity) {
	assert(model);
	model_ = model;

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;

	velocity_ = initialVelocity;

	// 他に必要な初期化処理があれば追加
	lifeTime_ = kLifeTime;
	isDead_ = false;
}

void HomingArcBullet::Update() {
	time_ += 1.0f;

	// 時間を0〜1に正規化
	float t = time_ / lifeTime_;
	if (t >= 1.0f) {
		isDead_ = true;
		return;
	}

	// 放物線の軌道を計算（中間で少し上に上がる）
	Vector3 control = (start_ + target_) * 0.5f + Vector3(0, 3.0f, 0); // 弧の高さ
	Vector3 p1 = Lerp(start_, control, t);
	Vector3 p2 = Lerp(control, target_, t);
	worldTransform_.translation_ = Lerp(p1, p2, t);

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();
}

void HomingArcBullet::Draw(const KamataEngine::Camera& camera) {
	if (!model_)
		return;
	model_->Draw(worldTransform_, camera);
}
