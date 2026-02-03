#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class TutorialUI {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新
	/// </summary>
	/// <param name="playerWorldPos">プレイヤーの3D座標</param>
	/// <param name="camera">現在のカメラ</param>
	void Update(const Vector3& playerWorldPos, const Camera& camera);

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	// 3D座標をスクリーン座標(2D)に変換するヘルパー関数
	Vector2 WorldToScreen(const Vector3& worldPos, const Camera& camera);

private:
	// スプライト
	uint32_t textureHandle_ = 0;
	Sprite* sprite_ = nullptr;

	// UIの中心座標（画面上の位置）
	Vector2 uiPosition_ = {900.0f, 550.0f}; // 画面右下あたり（画面サイズに合わせて調整してください）

	// フェード用パラメータ
	float currentAlpha_ = 1.0f;          // 現在の透明度 (0.0:透明 ~ 1.0:不透明)
	const float kMaxAlpha_ = 1.0f;       // 通常時の濃さ
	const float kMinAlpha_ = 0.2f;       // 重なった時の薄さ
	const float kFadeDistance_ = 200.0f; // このピクセル距離まで近づいたら薄くし始める
};