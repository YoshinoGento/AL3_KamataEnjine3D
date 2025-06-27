#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Enemy {
public:
	// 02_09 スライド5枚目
	void Initialize(Model* model, Camera* camera, const Vector3& position);
	// 02_09 スライド5枚目
	void Update();
	// 02_09 スライド5枚目
	void Draw();

private:

	// 02_09 6枚目 ワールド
	WorldTransform worldTransform_;

	// 02_09 6枚目 モデル
	Model* model_ = nullptr;

	// 02_09 6枚目 カメラ
	Camera* camera_ = nullptr;

	// 02_09 15枚目 歩行の速さ 
	static inline const float kWalkSpeed = 0.02f;

	// 02_09 15枚目 速度
	Vector3 velocity_ = {};

};
