#pragma once
#include "Bullet.h"
#include "Enemy.h"
#include "KamataEngine.h"
#include "Player.h"
#include <vector> // 複数管理に使う

using namespace KamataEngine;

class GameScene {
public:
	// 初期化
	void Initialize();

	// 終了処理
	void Delete();

	// 更新
	void Update();

	// 描画
	void Draw();


private:
	// プレイヤー
	Player* player_ = nullptr;

	// 雑魚敵
	std::vector<Enemy*> enemies_;

	// 弾（複数想定）
	std::vector<Bullet*> bullets_;
};
