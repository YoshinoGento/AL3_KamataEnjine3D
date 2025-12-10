#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Skydome {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(Model* model, Camera* camera);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 色のティントを設定（第二形態で少し赤くする用）
	/// </summary>
	void SetTintColor(const Vector4& color);

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	// カメラ
	Camera* camera_ = nullptr;
	// 色乗算用
	ObjectColor color_;
};
