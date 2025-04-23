#include "GameScene.h"
#include "ImageHlp.h"

using namespace KamataEngine;

void GameScene::Initialize() {

	// ファイル名を指定してテクスチャを読み込む
	textureHandle_ = TextureManager::Load("./Resources/miyazaki.png");
}

void GameScene::Update() {}

void GameScene::Drow() {}
