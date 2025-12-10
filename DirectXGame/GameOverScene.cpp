#include "GameOverScene.h"
#include "KamataEngine.h"
#include <algorithm>

using namespace KamataEngine;

void GameOverScene::Initialize() {

	// -------------------------
	// 背景画像をロード
	// -------------------------
	uint32_t bgTex = TextureManager::Load("GameOver.png");

	 auto* audio = Audio::GetInstance();
	bgmDataHandle_ = audio->LoadWave("GaneOver.wav"); // ←スペルも一応確認ね
	bgmVoiceHandle_ = audio->PlayWave(bgmDataHandle_, true);


	background_ = Sprite::Create(bgTex, {0, 0});
	background_->SetAnchorPoint({0, 0});

	// ★画像サイズ（必要なら実際のサイズに変更）
	float imageW = 1280.0f;
	float imageH = 720.0f;

	float screenW = 1280.0f;
	float screenH = 720.0f;

	float scale = (std::max)(screenW / imageW, screenH / imageH);

	background_->SetSize({imageW * scale, imageH * scale});
	background_->SetPosition({(screenW - imageW * scale) * 0.5f, (screenH - imageH * scale) * 0.5f});

	// -------------------------
	// PRESS KEY
	// -------------------------
	uint32_t pressTex = TextureManager::Load("presskeyDied.png");

	pressKey_ = Sprite::Create(pressTex, {640, 550});
	pressKey_->SetAnchorPoint({0.5f, 0.5f});
	pressKey_->SetSize({550, 120});
}

void GameOverScene::Update() {

	flashTimer_++;

	// 点滅
	if ((flashTimer_ / 20) % 2 == 0)
		pressKey_->SetColor({1, 1, 1, 1});
	else
		pressKey_->SetColor({1, 1, 1, 0});

	// ★ SPACE でタイトルへ戻る
	if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
		isEnd_ = true;
		nextScene_ = (int)SceneType::TITLE;
	}
}

void GameOverScene::Draw2D() {
	background_->Draw();
	pressKey_->Draw();
}

void GameOverScene::Finalize() {
	Audio::GetInstance()->StopWave(bgmVoiceHandle_);
	delete background_;
	delete pressKey_;
}
