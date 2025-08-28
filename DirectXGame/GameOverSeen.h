#pragma once
#include "Fade.h"
#include "KamataEngine.h"
#include "skydome.h"
using namespace KamataEngine;


class GameOverSeen {
public:
	// 02_12 27枚目 シーンのフェーズ
	enum class Phase {
		kFadeIn,  // フェードイン
		kMain,    // メイン部
		kFadeOut, // フェードアウト
	};

	~GameOverSeen();

	void Initialize();

	void Update();

	void Draw();

	// 02_12 26枚目
	bool IsFinished() const { return finished_; }

private:
	static inline const float kTimeTitleMove = 2.0f;

	// 02_03 天球
	Skydome* skydome_ = nullptr;
	Model* modelSkydome_ = nullptr;



	// ビュープロジェクション
	Camera camera_;
	WorldTransform worldTransformTitle_;
	WorldTransform worldTransformPlayer_;

	Model* modelPlayer_ = nullptr;
	Model* modelTitle_ = nullptr;

	float counter_ = 0.0f;
	// 02_12 26枚目
	bool finished_ = false;

	// 02_13 12枚目
	Fade* fade_ = nullptr;

	// 02_13 27枚目 現在のフェーズ
	Phase phase_ = Phase::kFadeIn;

};
