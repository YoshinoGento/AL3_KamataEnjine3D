#pragma once
#include "KamataEngine.h"
#include "Player.h"
#include <memory> // スマートポインタを使うなら

using namespace KamataEngine;

// ゲームシーン
class GameScene {
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// デストラクタ
	void Delete();

private:
	// プレイヤー
	Player* player_ = nullptr;

	// プレイヤー用モデル
	Model* playerModel_ = nullptr;

	// カメラ
	Camera camera_;

	// デバッグカメラ
	DebugCamera* debugCamera_ = nullptr;

	// デバッグカメラの有効フラグ
	bool isDebugCameraActive_ = false;
};
