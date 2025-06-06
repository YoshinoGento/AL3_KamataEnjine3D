#pragma once
#include "KamataEngine.h"

// 前方宣言
class Player;
using namespace KamataEngine;

/// <summary>
/// カメラコントロール
/// </summary>
class CameraController {
public:

	void Initialize();

	void Update();

	void SetTraget(Player* target) { target_ = target; }//スライド10枚

	

	private:
	// カメラ
	Camera *camera_ = nullptr;

	Player* target_ = nullptr;
};
