#pragma once  
#include "KamataEngine.h"  
#include "MapChipField.h"  
#include "Player.h" // Ensure this include is present and correct  
#include <vector>  
using namespace KamataEngine;  

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

void GenerateBlocks();  

private:  
// テクスチャーハンドル  
uint32_t textureHandle_ = 0;  

// サウンドデータハンドル  
uint32_t soundDataHandle_ = 0;  

// スプライト  
KamataEngine::Sprite* sprite_ = nullptr;  

// 3Dブロックモデル  
KamataEngine::Model* block_model_ = nullptr;  
std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;  

// 3Dモデル  
KamataEngine::Model* model_ = nullptr;  

// ワールドトランスフォーム  
KamataEngine::WorldTransform worldTransform_;  
// カメラ  
KamataEngine::Camera camera_;  

// ImGuiで値を入力する変数  
float inputFloat3[3] = {0, 0, 0};  

// デバックカメラ  
KamataEngine::DebugCamera* debugCamera_ = nullptr;  

// デバックカメラ有効  
bool isDebugCameraActive_ = false;  

// マップチップフィールド  
MapChipField* mapChipField_;  

	// プレイヤー
Player* player_ = nullptr;

KamataEngine::Model* modelPlayer_ = nullptr;
};
