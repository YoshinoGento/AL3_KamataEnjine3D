#include "BeamCharger.h"
#include "MatrixMath.h"
#include <algorithm> // clamp用

void BeamCharger::Initialize() { 

	absorbPower_ = 0.0f;
}



void BeamCharger::Update() {
	// 必要があればここにエフェクト更新などを追加
	
}

void BeamCharger::Draw(const Camera& camera) {
	(void)camera;

	// モデルが未読み込みのときだけ読み込む
	if (!barModel_) {
		barModel_ = Model::CreateFromOBJ("ChargeBar");

		// 読み込み失敗チェック
		if (!barModel_) {
			OutputDebugStringA("ChargeBar モデル読み込み失敗！\n");
			return;
		}
	}

	// 使用するチャージ率を取得
	float rate = GetChargeRate(); // 0.0～1.0
	if (rate <= 1.0f)
		return;

	// ワールドトランスフォーム設定
	static WorldTransform barWT; // ← スタックではなく static にして破棄されないように
	barWT.Initialize();
	barWT.translation_ = {-5.5f, 3.5f, 0.0f};
	barWT.scale_ = {rate * 2.0f, 0.2f, 1.0f};
	WorldTransformUpdate(barWT);

	barModel_->Draw(barWT, camera);
}

void BeamCharger::Absorb(float power) {
	absorbPower_ = std::clamp(absorbPower_ + power, 0.0f, maxPower_); 
}

float BeamCharger::GetChargeRate() const { return absorbPower_ / maxPower_; }

float BeamCharger::Consume() {
	float power = absorbPower_;
	absorbPower_ = 0.0f;
	return power;
}

bool BeamCharger::IsFull() const { return absorbPower_ >= maxPower_; }

