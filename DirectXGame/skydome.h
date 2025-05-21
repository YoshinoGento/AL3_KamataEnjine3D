#pragma once
#include "KamataEngine.h"
#include"Math.h"

/// <summary>
/// 天球
/// </summary>
class skydome {
public:

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Model* model,KamataEngine::Camera* camera);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();


	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

private:
	//ワールド変数データ
	KamataEngine::WorldTransform worldTransform_;
	//モデル
	KamataEngine::Model* model_ = nullptr;

	//カメラ
	KamataEngine::Camera* camera_ = nullptr;

	//数学関数
	Math math_;
};
