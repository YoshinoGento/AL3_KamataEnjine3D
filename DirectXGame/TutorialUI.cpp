#include "TutorialUI.h"
#include "MatrixMath.h" // これがないと計算できません
#include <algorithm>    // for std::clamp

void TutorialUI::Initialize() {
	// 操作説明の画像を読み込む
	textureHandle_ = TextureManager::Load("tutorial.png");

	// スプライト生成（位置は仮で0,0）
	sprite_ = Sprite::Create(textureHandle_, {0, 0});
}

void TutorialUI::Update(const Vector3& playerWorldPos, const Camera& camera) {
	// 1. プレイヤーの3D座標を、画面上の2D座標(スクリーン座標)に変換
	// MatrixMathに追加してもらう機能や、既存の機能を使います
	Vector2 playerScreenPos = WorldToScreen(playerWorldPos, camera);

	// 2. UIの位置とプレイヤーの位置の距離を測る
	float distance = Length(playerScreenPos - uiPosition_);

	// 3. 距離に応じて透明度を計算
	// 距離が kFadeDistance_ より近ければ薄くする
	float t = distance / kFadeDistance_;
	t = std::clamp(t, 0.0f, 1.0f); // 0~1に制限

	// 線形補間でアルファ値を決める (近いほどMin、遠いほどMax)
	currentAlpha_ = kMinAlpha_ + (kMaxAlpha_ - kMinAlpha_) * t;

	// 4. スプライトに反映
	if (sprite_) {
		sprite_->SetPosition({uiPosition_.x, uiPosition_.y});
		// 色設定（白で、アルファ値だけ変える）
		sprite_->SetColor({1.0f, 1.0f, 1.0f, currentAlpha_});
	}
}

void TutorialUI::Draw() {
	if (sprite_) {
		sprite_->Draw();
	}
}

Vector2 TutorialUI::WorldToScreen(const Vector3& worldPos, const Camera& camera) {
	// KamataEngineのCamera変数は public なので直接アクセスします
	// GetViewMatrix() ではなく .matView を使います

	// ★画面サイズは固定か、WinAppから取得してください
	const float kScreenWidth = 1280.0f;
	const float kScreenHeight = 720.0f;

	// MatrixMath にある ProjectToScreen があればそれを使えますが、
	// 念のためここで Multiply を使って確実に計算します。

	// 行列の掛け算（MatrixMathに追加してもらう関数）
	Matrix4x4 matViewProj = Multiply(camera.matView, camera.matProjection);

	// ビュープロジェクション変換 (MatrixMathにある関数)
	Vector3 ndc = Transform(worldPos, matViewProj);

	// NDC(-1~1) を スクリーン座標に変換
	Vector2 screenPos;
	screenPos.x = (ndc.x + 1.0f) * 0.5f * kScreenWidth;
	screenPos.y = (1.0f - ndc.y) * 0.5f * kScreenHeight;

	return screenPos;
}