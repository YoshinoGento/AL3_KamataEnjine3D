#include "Barrier.h"
#include <algorithm>

void Barrier::Initialize() {
	durability_ = kMaxDurability;
	coolTime_ = 0.0f;
	model_ = Model::CreateFromOBJ("BarrierSphere"); // 半透明天球モデル


	if (!model_) {
		OutputDebugStringA("BarrierSphere モデルの読み込みに失敗しました\n");
	}

	// 🔽 追加：ObjectColor初期化
	objectColor_.Initialize();
	objectColor_.SetColor(color_); // <- ここでBarrierの色を反映
	worldTransform_.Initialize();
}

void Barrier::Update() {
	if (isBroken_) {
		coolTime_ -= 1.0f;
		if (coolTime_ <= 0.0f) {
			isBroken_ = false;
			durability_ = kMaxDurability;
		}
	}

	worldTransform_.translation_ = {0, 0, 0}; // playerPosition;
	worldTransform_.scale_ = {1.5f, 1.5f, 1.5f};
	WorldTransformUpdate(worldTransform_);
}

void Barrier::Absorb(float power) {
	if (isBroken_)
		return;

	durability_ -= power;
	if (durability_ <= 0.0f) {
		durability_ = 0.0f;
		isBroken_ = true;
		coolTime_ = kCoolTime; // 例：クールタイム 3秒
	}
}

void Barrier::OnBeamFired() {
	if (!isBroken_) {
		durability_ = kMaxDurability;
	}
}

bool Barrier::IsBroken() const { return isBroken_; }

bool Barrier::IsCoolingDown() const { return isBroken_ && coolTime_ > 0.0f; }

float Barrier::GetDurabilityRate() const { return durability_ / kMaxDurability; }

void Barrier::Draw(const Camera& camera, const Vector3& playerPosition) {
	(void)playerPosition; // 未使用警告防止
	if (isBroken_ || !model_) {
		return;
	}

	float rate = this->GetDurabilityRate();

	// 色をLerpでスムーズに変化
	color_ = Lerp({0.2f, 0.6f, 1.0f, 0.2f}, {1.0f, 0.2f, 0.2f, 0.4f}, 1.0f - rate);

	// オブジェクトカラーに色をセット
	objectColor_.SetColor(color_);
	model_->Draw(worldTransform_, camera, &objectColor_);
}
