#include "TitleScene.h"
#include "KamataEngine.h"
#include <algorithm> // Ensure this header is included for std::max

using namespace KamataEngine;

void TitleScene::Initialize() {
	uint32_t titleTex = TextureManager::Load("TitleScene.png");

	auto* audio = Audio::GetInstance();
	bgmDataHandle_ = audio->LoadWave("TitleScene.wav");
	bgmVoiceHandle_ = audio->PlayWave(bgmDataHandle_, true);

	title_ = Sprite::Create(titleTex, {0, 0});

	// -------------------------
	// フルスクリーンフィット計算
	// -------------------------
	// 本来は TextureManager から画像サイズを取得できるが、
	// ここでは仮の値として画像のピクセルサイズを入れる（要確認）
	float imageW = 1280.0f; // ← TitleScene.png の幅に置き換えて
	float imageH = 720.0f;  // ← TitleScene.png の高さに置き換えて

	float screenW = 1280.0f;
	float screenH = 720.0f;

	// Fix: Ensure std::max is used with the correct namespace
	float scale = (std::max)(screenW / imageW, screenH / imageH);

	float finalW = imageW * scale;
	float finalH = imageH * scale;

	title_->SetSize({finalW, finalH});

	// 中央揃え（上に詰まっているのも修正）
	title_->SetPosition({(screenW - finalW) * 0.5f, (screenH - finalH) * 0.5f});

	// press key
	uint32_t pressTex = TextureManager::Load("presskey.png");
	pressKey_ = Sprite::Create(pressTex, {640, 500});
	pressKey_->SetAnchorPoint({0.5f, 0.5f});
	pressKey_->SetSize({550, 120});
}

void TitleScene::Update() {

	flashTimer_++;

	// 点滅（20フレームごとに ON/OFF）
	if ((flashTimer_ / 20) % 2 == 0) {
		pressKey_->SetColor({1, 1, 1, 1});
	} else {
		pressKey_->SetColor({1, 1, 1, 0});
	}

	// スペースキーでゲームシーンへ
	if (Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		isEnd_ = true;
		nextScene_ = (int)SceneType::GAME;
	}
}

void TitleScene::Draw2D() {
	title_->Draw();
	pressKey_->Draw();
}

void TitleScene::Finalize() {
	delete title_;
	delete pressKey_;
	Audio::GetInstance()->StopWave(bgmVoiceHandle_);
}
