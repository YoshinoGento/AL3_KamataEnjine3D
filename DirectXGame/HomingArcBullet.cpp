#include "HomingArcBullet.h"
#include "KamataEngine.h"
#include "MatrixMath.h"
#include <cassert>
#include <algorithm>

using namespace KamataEngine;

void HomingArcBullet::Initialize(Model* model, const Vector3& start, const Vector3& target, const Vector3& controlOffset) {
	assert(model);
	model_ = model;
	worldTransform_.Initialize();
	worldTransform_.translation_ = start;

	start_ = start;
	target_ = target;
	controlOffset_ = controlOffset; // これで個別に！

	time_ = 0.0f;
	lifeTime_ = kLifeTime;
	isDead_ = false;

	// Initialize の最後に追加
	prevPosition_ = worldTransform_.translation_;
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
	// float distance = Length(target_ - start_);
	Vector3 control = mid + controlOffset_; // ←ここで offset が効いてないと意味ない！

	// ベジェ補間
	Vector3 p1 = Lerp(start_, control, t);
	Vector3 p2 = Lerp(control, target_, t);
	worldTransform_.translation_ = Lerp(p1, p2, t);

	WorldTransformUpdate(worldTransform_);
	worldTransform_.TransferMatrix();


	// 進行方向ベクトル（前の位置との差）
	Vector3 velocity = worldTransform_.translation_ - prevPosition_;
	if (Length(velocity) > 0.001f) {
		Vector3 forward = Normalized(velocity);

		// 基準方向（モデルの正面がZ+として）との向きの差を軸に回転を計算
		Vector3 defaultForward = {0, 0, 1};                    // モデルの元の前向き
		Vector3 rotationAxis = Cross(defaultForward, forward); // 外積で回転軸
		float dot = Dot(defaultForward, forward);              // 内積で角度
		float angle = std::acos(std::clamp(dot, -1.0f, 1.0f)); // 角度に変換

		if (Length(rotationAxis) > 0.0001f) {
			rotationAxis = Normalized(rotationAxis);

			// 回転行列を作って回転を適用
			Matrix4x4 rotMat = MakeRotateAxisMatrix(rotationAxis, angle);
			worldTransform_.rotation_ = GetEulerFromMatrix(rotMat);
		}
	}


	// HomingArcBullet::Update() の中などで
	if (Length(worldTransform_.translation_ - target_) < 1.0f) {
		isDead_ = true; // 弾を消す
		                // ここで敵のHPを減らしたり、爆発出したりできる！
	}

	if (static_cast<int>(time_) % 2 == 0 && time_ < 40.0f) {
		MissilePartocle* p = new MissilePartocle();
		p->Initialize(worldTransform_.translation_);
		missileParticles_.push_back(p);
	}



	for (auto* p : missileParticles_) {
		p->Update();
	}

	// missileParticles_ の削除
	missileParticles_.remove_if([](MissilePartocle* p) {
		if (p->IsDead()) {
			delete p;
			return true;
		}
		return false;
	});

	// Update の最後に追加
	prevPosition_ = worldTransform_.translation_;
}

void HomingArcBullet::Draw(const KamataEngine::Camera& camera) {
	if (!model_)
		return;
	model_->Draw(worldTransform_, camera);
	for (auto* p : missileParticles_) {
		p->Draw(camera);
	}
}
