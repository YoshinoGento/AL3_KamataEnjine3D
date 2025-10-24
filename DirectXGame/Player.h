#pragma once
#include "PlayerBullet.h"
#include "KamataEngine.h"
#include <list>

using namespace KamataEngine;

class Player {
public:
	/// 初期化
	void Initialize(Model* model, Camera* camera, const Vector3& position);

	/// 更新
	void Update();

	/// 描画
	void Draw();

	/// <summary>
	/// 攻撃
	/// </summary>
	void Attack();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Player();

private:
	// ワールド変換データ
	WorldTransform worldTransform_;
	// モデル
	Model* model_ = nullptr;
	// テクスチャハンドル
	uint32_t textureHandle_ = 0u;
	// カメラ
	Camera* camera_ = nullptr;

	// 移動速度
	Vector3 velocity_ = {0, 0, 0};

	// 弾
	std::list<PlayerBullet*> bullets_;
	
	//キーボード入力
	Input* input_ = nullptr;
};