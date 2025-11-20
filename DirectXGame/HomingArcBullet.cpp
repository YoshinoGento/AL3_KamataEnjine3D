#include "HomingArcBullet.h"
#include "MatrixMath.h"
#include "KamataEngine.h"
#include <cassert>

using namespace KamataEngine;


void HomingArcBullet::Initialize(Model* model, const Vector3& start, const Vector3& target) {
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

	// 距離に応じて制御点の高さを変える
	Vector3 mid = (start_ + target_) * 0.5f;
	float distance = Length(target_ - start_);
	Vector3 control = mid + Vector3(0, distance * 0.5f, 0); // 弧を大きく！

	// ベジェ補間
	Vector3 p1 = Lerp(start_, control, t);
	Vector3 p2 = Lerp(control, target_, t);
	worldTransform_.translation_ = Lerp(p1, p2, t);

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();

	// HomingArcBullet::Update() の中などで
	if (Length(worldTransform_.translation_ - target_) < 1.0f) {
		isDead_ = true; // 弾を消す
		// ここで敵のHPを減らしたり、爆発出したりできる！
	}
}


void HomingArcBullet::Draw(const KamataEngine::Camera& camera) {
	if (!model_)
		return;
	model_->Draw(worldTransform_, camera);
}
