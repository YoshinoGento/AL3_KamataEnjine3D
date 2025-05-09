#include "GameScene.h"
#include "KamataEngine.h"
#include <Windows.h>
#include "Math.h"

// これを書くことでKamataEmgine::と書かずに済む
using namespace KamataEngine;

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	// エンジンの初期化
	KamataEngine::Initialize();

	// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	//ImGuiManagerインスタントの取得
	ImGuiManager* imguiManager = ImGuiManager::GetInstance();

	// ゲームシーンのインスタンス生成
	GameScene* gameScene = new GameScene();

	// ゲームシーンの初期化
	gameScene->Initialize();

	// メインループ
	while (true) {

		//ImGui受付開始
		imguiManager->Begin();

		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		//ゲームシーンの更新
		gameScene->Update();
	
		//ImGui受付終了
		imguiManager->End();

		// 描画開始
		dxCommon->PreDraw();

		//ゲームシーンの描画
		gameScene->Drow();

		AxisIndicator::GetInstance()->Draw();

		//ImGui描画
		imguiManager->Draw();

		// ここに描画処理を記述する

		// 描画終了
		dxCommon->PostDraw();


	}
	
	//解放する
	delete gameScene;
	gameScene = nullptr;
	
	// エンジンの終了処理
	KamataEngine::Finalize();
	return 0;
}
