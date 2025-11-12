#pragma once
#include "KamataEngine.h"
#include "Player.h"
#include "Enemy.h"
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
	// エネミー
	Enemy* enemy_ = nullptr;

	// プレイヤー用モデル
	Model* player_model_ = nullptr;
	// エネミー用モデル
	Model* enemy_model_ = nullptr;

	// カメラ
	Camera camera_;
	Camera PlayerCamera_;
	Camera EnemyCamera_;

	//テクスチャハンドル
	uint32_t textureHandle_ = 0u;


	// デバッグカメラ
	DebugCamera* debugCamera_ = nullptr;

	// デバッグカメラの有効フラグ
	bool isDebugCameraActive_ = false;

	
};