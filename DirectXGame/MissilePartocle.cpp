#include "MissilePartocle.h"
#include <cassert>
#include "MatrixMath.h"

void MissilePartocle::Initialize(const Vector3& position) {
	model_ = Model::CreateSphere();

	worldTransform_.Initialize();
	worldTransform_.translation_ = position;
	initialScale_ = {0.5f, 0.5f, 0.5f}; // 任意の大きさ
	worldTransform_.scale_ = initialScale_;
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};
	WorldTransformUpdate(worldTransform_);

	time_ = 0.0f;
	isDead_ = false;
}


void MissilePartocle::Update() {
	time_ += 1.0f;
	if (time_ >= lifetime_) {
		isDead_ = true;
		return;
	}

	float rate = (lifetime_ - time_) / lifetime_;

	// 初期スケールに対して割合を掛ける
	worldTransform_.scale_ = initialScale_ * rate;

	alpha_ = rate;

	WorldTransformUpdate(worldTransform_);
}


void MissilePartocle::Draw(const Camera& camera) {  
    if (!model_ || isDead_)  
        return;  

    // 修正: alpha_ を uint32_t にキャストして渡す  
    model_->Draw(worldTransform_, camera, static_cast<uint32_t>(alpha_));  
}