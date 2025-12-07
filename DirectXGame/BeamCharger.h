#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class BeamCharger {
public:
	void Initialize();
	void Update();
	void Draw(const Camera& camera); // ゲージ描画など

	void Absorb(float power);    // 吸収する
	float GetChargeRate() const; // 0.0〜1.0 の割合
	float Consume();             // 使用後リセットして値を返す
	bool IsFull() const;

private:

	float absorbPower_ = 0.0f;
	const float maxPower_ = 100.0f;
};
