#include "GameClearScene.h"
#include "GameOverSeen.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "TitleScene.h" // 02_12 21枚目
#include <Windows.h>

using namespace KamataEngine;

// 02_12 24枚目
TitleScene* titleScene = nullptr;
GameScene* gameScene = nullptr;
GameOverSeen* gameOverScene = nullptr;
GameClearScene* gameClearScene = nullptr;

// 02_12 25枚目(Scene sceneまで)
enum class Scene { kUnknown = 0, kTitle, kGame, kEnd, kClear };
// 現在シーン（型）
Scene scene = Scene::kUnknown;

// 02_12 29枚目
void ChangeScene() {

	switch (scene) {
	case Scene::kTitle:
		if (titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kGame;
			delete titleScene;
			titleScene = nullptr;
			gameScene = new GameScene;
			gameScene->Initialize();
		}
		break;
	case Scene::kGame:
		// 02_12 30枚目
		if (gameScene->IsFinished()) {
			if (gameScene->IsCleared()) { // GameSceneに勝利判定関数を追加

				// 勝利か敗北かで分岐
				scene = Scene::kClear;
				gameClearScene = new GameClearScene;
				gameClearScene->Initialize();

			} else {

				// シーン変更
				scene = Scene::kEnd;
				gameOverScene = new GameOverSeen;
				gameOverScene->Initialize();
			}

			delete gameScene;
			gameScene = nullptr;
		}

		break;
	case Scene::kEnd:
		// 02_12 30枚目
		if (gameOverScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;

			delete gameOverScene;
			gameOverScene = nullptr;
			delete gameClearScene;
			gameClearScene = nullptr;

			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;
	case Scene::kClear:
		// 02_12 30枚目
		if (gameClearScene->IsFinished()) {
			// シーン変更
			scene = Scene::kTitle;

			delete gameClearScene;
			gameClearScene = nullptr;

			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;
	}
}

// 02_12 31枚目
void UpdateScene() {

	switch (scene) {
	case Scene::kTitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;

	case Scene::kEnd:
		gameOverScene->Update();
		break;

	case Scene::kClear:
		gameClearScene->Update();
		break;
	}
}


// 02_12 32枚目
void DrawScene() {
	switch (scene) {
	case Scene::kTitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	case Scene::kEnd:
		gameOverScene->Draw();
		break;
	case Scene::kClear:
		gameClearScene->Draw();
		break;
	}
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	// エンジンの初期化
	KamataEngine::Initialize(L"LE2C_29_ヨシノ_ゲント_RunMaru");

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// ImGuiManagerインスタンスの取得
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	// 02_12 21枚目 ↓3行
	//	TitleScene *titleScene = nullptr; // 02_12 24枚目でグローバルに引っ越し
	scene = Scene::kTitle; // 02_12 28枚目
	titleScene = new TitleScene;
	titleScene->Initialize();

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		// ImGui受付開始
		imguiManager->Begin();

		// 02_12 21枚目で変更
		//		titleScene->Update(); //02_12 33枚目で削除

		// シーン切り替え
		ChangeScene(); // 02_12 33枚目で追加
		// シーン更新
		UpdateScene(); // 02_12 33枚目で追加

		// ImGui受付終了
		imguiManager->End();

		// 描画開始
		dxCommon->PreDraw();

		// ゲームシーンの描画
		//		titleScene->Draw(); // 02_12 33枚目で削除

		// シーンの描画
		DrawScene(); // 02_12 33枚目で追加

		// 軸表示の描画
		AxisIndicator::GetInstance()->Draw();

		// プリミティブ描画のリセット
		PrimitiveDrawer::GetInstance()->Reset();

		// ImGui描画
		imguiManager->Draw();
		imguiManager->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}

	// 02_12 35枚目 各種解放
	delete titleScene;
	delete gameScene;
	delete gameOverScene;

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}
