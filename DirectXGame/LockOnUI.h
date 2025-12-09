#pragma once
#include "Enemy.h"
#include "KamataEngine.h"



class LockOnUI {
public:
	static void Draw(const Enemy* enemy, const Camera* camera, int screenWidth, int screenHeight);
};
