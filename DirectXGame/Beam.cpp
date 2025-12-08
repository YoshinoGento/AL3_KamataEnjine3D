#include "Beam.h"
#include "MatrixMath.h"
#include <algorithm>



void Beam::Initialize(const Vector3& start, const Vector3& target, float chargePower) {
	model_ = Model::CreateFromOBJ("PlayerBeam");
	worldTransform_.Initialize();

	Vector3 dir = Normalized(target - start);
	float length = Length(target - start);

	// チャージ量（0～100）を 0.1～0.5 の太さにマッピング
	float thickness = 0.1f + (chargePower / 100.0f) * 0.4f;

	worldTransform_.scale_ = {thickness, thickness, length};
	worldTransform_.rotation_ = GetEulerFromMatrix(MakeLookRotation(dir));
	worldTransform_.translation_ = start;

	WorldTransformUpdate(worldTransform_);

	time_ = 0.0f;
	isDead_ = false;

	// ライフタイムをチャージに応じて少し伸ばす（任意）
	lifeTime_ = 20.0f + (chargePower / 100.0f) * 20.0f;
}






void Beam::Update() {
	time_++;
	if (time_ >= lifeTime_) {
		isDead_ = true;
		return;
	}

	// ビームの長さや透明度を時間で変えることも可能
	WorldTransformUpdate(worldTransform_);
}


void Beam::Draw(const Camera& camera) {
	if (!isDead_ && model_) {
		model_->Draw(worldTransform_, camera/*, static_cast<uint32_t>(0.8f)*/);
	}
}
