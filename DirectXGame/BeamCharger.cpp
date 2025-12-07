#include "BeamCharger.h"
#include <algorithm> // clamp用

void BeamCharger::Initialize() { 

	absorbPower_ = 0.0f; 

}

void BeamCharger::Update() {
	// 必要があればここにエフェクト更新などを追加
}

void BeamCharger::Draw(const Camera& camera) {
	// 今後、チャージゲージを描画するならここに書く
	(void)camera; // 未使用警告防止
}

void BeamCharger::Absorb(float power) { absorbPower_ = std::clamp(absorbPower_ + power, 0.0f, maxPower_); }

float BeamCharger::GetChargeRate() const { return absorbPower_ / maxPower_; }

float BeamCharger::Consume() {
	float power = absorbPower_;
	absorbPower_ = 0.0f;
	return power;
}

bool BeamCharger::IsFull() const { return absorbPower_ >= maxPower_; }
