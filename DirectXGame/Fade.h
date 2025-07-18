#pragma once
#include "KamataEngine.h"
class Fade {
public:
	void Initialize();
	void Update();
	void Draw();

private:
	//02_13 10枚目 スプライト
	KamataEngine::Sprite* sprite_ = nullptr;


};
