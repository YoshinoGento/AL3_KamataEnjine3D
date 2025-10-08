#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Player {
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Drow();

private:

	Model* model_ = nullptr; // モデルデータ

	WorldTransform worldTransform_; // 位置・回転・スケール
	
};
