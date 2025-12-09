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
	

	// クールダウン処理
	if (isInCooldown_) {
		cooldownTimer_--;
		if (cooldownTimer_ <= 0) {
			isInCooldown_ = false;
			durability_ = kMaxDurability; // ← バリア復活
			isBroken_ = false;            // ← これも忘れずに！
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
		Break(); // ← ★ ここを呼ぶ！
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
	if (isBroken_ || !model_) {
		return;
	}

	float rate = this->GetDurabilityRate();

	// 色をLerpでスムーズに変化
	// 透明度をもっと薄くして、敵が見えるように調整
	color_ = Lerp({0.2f, 0.6f, 1.0f, 0.05f}, {1.0f, 0.2f, 0.2f, 0.15f}, 1.0f - rate);
	objectColor_.SetColor(color_);

	// ★ プレイヤーの位置を追従させる（Z方向にオフセットもあり？）
	worldTransform_.translation_ = playerPosition;
	WorldTransformUpdate(worldTransform_); // ←これ重要！

	model_->Draw(worldTransform_, camera, &objectColor_);

}


void Barrier::Reset() {
	durability_ = kMaxDurability;
	isBroken_ = false;
	isInCooldown_ = false;
	cooldownTimer_ = 0;
}


void Barrier::Break() {
	isBroken_ = true;
	isInCooldown_ = true;
	cooldownTimer_ = kCooldownTime;
}

bool Barrier::IsInCooldown() const { return isInCooldown_; }
