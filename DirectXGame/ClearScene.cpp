#include "ClearScene.h"
#include "KamataEngine.h"
#include <algorithm>

using namespace KamataEngine;

void ClearScene::Initialize() {

	// ================================
	// GameClear の背景画像を読み込む
	// ================================
	uint32_t tex = TextureManager::Load("GameClear.png");
	clearText_ = Sprite::Create(tex, {0, 0});

	// -------------------------
	// フルスクリーンフィット計算
	// -------------------------
	// ★ 本当は画像サイズを自動取得するのが理想だけど、
	//    TitleScene と同じく数値を入れる方式で合わせる
	float imageW = 1280.0f; // ← GameClear.png の元の幅に合わせてね
	float imageH = 720.0f;  // ← GameClear.png の元の高さに合わせる

	float screenW = 1280.0f;
	float screenH = 720.0f;

	float scale = (std::max)(screenW / imageW, screenH / imageH);

	float finalW = imageW * scale;
	float finalH = imageH * scale;

	clearText_->SetSize({finalW, finalH});
	clearText_->SetPosition({(screenW - finalW) * 0.5f, (screenH - finalH) * 0.5f});

	// ================================
	// PRESS KEY の表示
	// ================================
	uint32_t pressTex = TextureManager::Load("presskeyClear.png");
	pressKey_ = Sprite::Create(pressTex, {640, 550});

	pressKey_->SetAnchorPoint({0.5f, 0.5f});
	pressKey_->SetSize({550, 120}); // TitleScene と同じ大きさ
}

void ClearScene::Update() {

	// -------------------------
	// 点滅処理
	// -------------------------
	flashTimer_++;
	if ((flashTimer_ / 20) % 2 == 0) {
		pressKey_->SetColor({1, 1, 1, 1});
	} else {
		pressKey_->SetColor({1, 1, 1, 0});
	}

	// -------------------------
	// スペースキー → タイトルへ戻る
	// -------------------------
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		isEnd_ = true;
		nextScene_ = (int)SceneType::TITLE;
	}
}

void ClearScene::Draw2D() {
	clearText_->Draw();
	pressKey_->Draw();
}

void ClearScene::Finalize() {
	delete clearText_;
	delete pressKey_;
}
