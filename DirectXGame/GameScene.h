#pragma once
#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include "Scene.h"
#include "SceneType.h"
#include "Skydome.h"
#include <memory> // スマートポインタを使うなら

using namespace KamataEngine;


enum class GameState {
	Play,  // 通常プレイ
	Pause, // ポーズ中
};

class GameScene : public Scene {
public:
	void Initialize() override;
	void Update() override;
	void Draw3D() override;
	void Draw2D() override; // ← UI ここに移動
	void Finalize() override;

	bool IsEnd() override { return isEnd_; }
	int NextScene() override { return nextScene_; }

private:
	Player* player_ = nullptr;
	Enemy* enemy_ = nullptr;
	Model* player_model_ = nullptr;
	Model* enemy_model_ = nullptr;
	Model* player_bullet_model_ = nullptr;

	Camera camera_;
	Camera PlayerCamera_;
	Camera EnemyCamera_;

	uint32_t textureHandle_ = 0u;
	DebugCamera* debugCamera_ = nullptr;
	bool isDebugCameraActive_ = false;

	Skydome* skydome_ = nullptr;
	Model* skydome_model_ = nullptr;

	bool isEnd_ = false;
	int nextScene_ = (int)SceneType::CLEAR;

	// GameScene.h のメンバ
	uint32_t bgmDataPhase1_ = 0;
	uint32_t bgmDataPhase2_ = 0;
	uint32_t bgmVoiceHandle_ = 0;

	Enemy::Form lastForm_ = Enemy::Form::ONE;

	// プレイヤーの体力のモデル
	Model* playerHealth_model_ = nullptr;
	// プレイヤーの体力バーのワールド変換
	WorldTransform worldTransformPlayerHealth_;

	// 敵の体力のモデル
	Model* enemyHealth_model_[3] = {nullptr};
	// 敵の体力バーのワールド変換
	WorldTransform worldTransformEnemyHealth_[3];

	// 体力バーのテクスチャ
	uint32_t health_texture;
	ObjectColor playerHealthBarColor;
	ObjectColor enemyHealthBarColor;

	// --- ポーズメニュー ---
	bool isPaused_ = false; // ←もう不要になるけど一旦残してOK

	// ★ 追加する！ ★
	GameState state_ = GameState::Play;

	Sprite* menuBG_ = nullptr;
	Sprite* highlight_ = nullptr;

	Sprite* continueText_ = nullptr;
	Sprite* titleText_ = nullptr;

	Sprite* cursor_ = nullptr;

	// 選択項目（0=つづける, 1=タイトルへ）
	int menuIndex_ = 0;
	// -------------------------------

	/// UI追加
	// --- 操作説明UI ---
	Sprite* controlUI_ = nullptr;

	// 表示タイマー
	int controlUITimer_ = 0;

	// フェードアウト用（0.0〜1.0）
	float controlUIAlpha_ = 1.0f;

	// 今UIが表示されるべきか？
	bool showControlUI_ = true;



};
