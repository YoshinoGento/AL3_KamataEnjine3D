#include "Beam.h"
#include "MatrixMath.h"
#include <algorithm>



void Beam::Initialize(const Vector3& start, const Vector3& target) {
	model_ = Model::CreateFromOBJ("PlayerBeam");
	worldTransform_.Initialize();

	Vector3 dir = Normalized(target - start);
	float length = Length(target - start);
	float thickness = 0.1f; // ← 見た目の厚みを「長さの1割」にする

	// スケール（Z方向に長いビーム）
	worldTransform_.scale_ = {thickness, thickness, length};

	// 回転（Z+方向をdirに）
	worldTransform_.rotation_ = GetEulerFromMatrix(MakeLookRotation(dir));

	// ✅ モデルの中心が真ん中なら、Z+に半分ずらすことで「startから先に」伸びる
	//worldTransform_.translation_ = start + dir * (2.0f * 0.5f * length); // Blender実サイズ2.0 × スケール
	worldTransform_.translation_ = start; // ← これだけでOK



	WorldTransformUpdate(worldTransform_);

	time_ = 0.0f;
	isDead_ = false;
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
