#pragma once
#include "KamataEngine.h"
/// <summary>
/// 自キャラ
/// </summary>
class  Player {
public:
	 Player();
	~ Player();

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

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
	KamataEngine::WorldTransform worldTransfrom_;

	//モデル
	KamataEngine::Model* model_ = nullptr;

	//テクスチャハンドル
	uint32_t textureHandle_ = 0u;


};



