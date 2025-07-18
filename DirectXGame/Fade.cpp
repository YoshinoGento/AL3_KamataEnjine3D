#include "Fade.h"
using namespace KamataEngine;

void Fade::Initialize() { 

	// 02_13 10枚目 スプライトの生成
	sprite_ = Sprite::Create(0, Vector2{});
	sprite_->SetSize(Vector2(WinApp::kWindowWidth, WinApp::kWindowHeight));
	sprite_->SetColor(Vector4(0, 0, 0, 1));

}

void Fade::Draw() {

	// 02_13 11枚目 スプライト描画
	Sprite::PreDraw(DirectXCommon::GetInstance()->GetCommandList());
	sprite_->Draw();
	Sprite::PostDraw();
}