#pragma once
#include "KamataEngine.h"
#include <vector>

// ゲームシーン
class GameScene {
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Drow();


	~GameScene();


private:
	// テクスチャーハンドル
	uint32_t textureHandle_ = 0;

	//サウンドデータハンドル
	uint32_t soundDataHandle_ = 0;

	////音声再生ハンドル
	//uint32_t voiceHandle_ = 0;

	//スプライト
	KamataEngine::Sprite* sprite_ = nullptr;

	//3Dブロックモデル
	KamataEngine::Model* block_model_ = nullptr;
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// 3Dモデル
	KamataEngine::Model* model_ = nullptr;
	

	//ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;
	//カメラ
	KamataEngine::Camera camera_;

	//ImGuiで値を入力する変数
	float inputFloat3[3] = {0, 0, 0};

	//デバックカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	//デバックカメラ有効
	bool isDebugCameraActive_ = false;
	

};

